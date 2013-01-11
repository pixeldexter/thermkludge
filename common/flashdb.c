/*
 * Minimum flash translation layer.
 * No redundancy, no wear leveling, no real fs, single file
 * can only write even number bytes at a time
 */

#include "flashdb.h"
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>



/* flags */

#define USER_OPENFLAGS_MASK 0xF
#define SF_OPEN (1<<4)
#define SF_DIRTY (1<<7)


/* pages */
#define PAGE_MAX 96 // for m8, TODO: determine amount of usable pages somehow
#define SPM_PAGESIZE_W (SPM_PAGESIZE / 2)
#define SPM_BITMAPSIZE (SPM_PAGESIZE_W / 8)
#define BITS_PER_BYTE 8

struct page_header
{
	uint8_t bitmap[SPM_BITMAPSIZE];
	uint8_t data[0];
}__attribute__((packed));

typedef uint8_t page_ofs_t;

struct page_openpage
{
	const prog_uint8_t *pgm_address;
	page_ofs_t write_offset;
	struct page_header header;
};

/* streams */

#define MAX_STREAMS 1

struct stream
{
	struct page_openpage wpage; /* page currently opened for writing */
	uint8_t flags;
};

struct stream streams[MAX_STREAMS];



/* code */

static page_ofs_t __find_write_offset(struct page_openpage *page)
{
	const page_ofs_t max_count = SPM_PAGESIZE;
	page_ofs_t offset;

	for(offset = sizeof(struct page_header);
	    offset != max_count;
	    offset += 2)
	{
		const unsigned bit_byte = offset
			/ sizeof(uint16_t)
			/ BITS_PER_BYTE
			;

		const unsigned bit_bit = offset
			/ sizeof(uint16_t)
			% BITS_PER_BYTE
			;

		if ( page->header.bitmap[ bit_byte ] & (1<<bit_bit) )
		{
			// TODO: ensure there are 0xfff... till the end of bitmap
			// for now, just assume it is there
			break;
		}
	}
	return offset;
}

#define NOINLINE __attribute__((noinline))

static int __page_is_dirty(struct page_openpage *page)
{
	((void)page); // suppress warning
	return 1;
}

static void __page_read_header(struct page_openpage *page)
{
#if SPM_BITMAPSIZE == 4 && 0
	__asm__ (
		"1: lpm __tmp_reg__, %a0+" "\n\t"
		"st  %a1+,__tmp_reg__"     "\n\t"
		"dec %2"                   "\n\t"
		"brne 1b"
		:
		: "z" (page->pgm_address),
		  "e" (&page->header),
		  "r" (4)
		);
#else
	memcpy_P(&page->header, page->pgm_address, sizeof(struct page_header));
#endif
}

static void __page_commit_header(const struct page_openpage *page)
{
	uint16_t *adr_dst = (uint16_t*)(page->pgm_address);
	const uint16_t *adr_src = (const void *)&page->header;
	uint8_t word_count = SPM_BITMAPSIZE / sizeof(uint16_t);

	while ( word_count-- )
		boot_page_fill(adr_dst++, *adr_src++);
}

static NOINLINE void __page_flush(const struct page_openpage *page)
{
	__page_commit_header(page);
	boot_page_write(page->pgm_address);
}

static NOINLINE void __page_open(struct page_openpage *page, uint16_t addr)
{
	page->pgm_address = (PGM_VOID_P)addr;
	page->write_offset = sizeof(struct page_header);
	__page_read_header(page);
}

static NOINLINE void __page_close(struct page_openpage *page)
{
	if ( __page_is_dirty(page) )
	{
		__page_flush(page);
		boot_spm_busy_wait();
	}
	page->pgm_address = (void*)(~0);
}

static NOINLINE void __page_erase(uint16_t addr)
{
	boot_page_erase(addr);
	boot_spm_busy_wait();
}

#ifdef SUPPORT_MULTISTREAM
flashdb_return_type flashdb_open(uint8_t stream, uint8_t flags)
#else
flashdb_return_type flashdb_open(uint8_t flags)
#endif
{
#ifndef SUPPORT_MULTISTREAM
	const uint8_t stream = 0;
#endif

	if ( stream >= MAX_STREAMS )
		return -EBADF;

	struct stream * const desc = streams + stream;

	if ( desc->flags & SF_OPEN )
		return -EBUSY;

	desc->flags = 0
		| SF_OPEN
		| (flags & USER_OPENFLAGS_MASK)
		;

	uint16_t page = 0;
	do {
		__page_open(&desc->wpage, page);
		desc->wpage.write_offset = __find_write_offset(&desc->wpage);
		page += SPM_PAGESIZE;
	} while ( desc->wpage.write_offset >= SPM_PAGESIZE );
	return stream;
}

#ifdef SUPPORT_MULTISTREAM
flashdb_return_type flashdb_write(uint8_t stream_id, const void *buf, size_t sz)
#else
flashdb_return_type flashdb_write(const void *buf, size_t sz)
#endif
{
#ifndef SUPPORT_MULTISTREAM
	const uint8_t stream_id = 0;
#endif

	// TODO: sanity check
	struct stream * const s = streams + stream_id;

	const uint16_t * pdata = (const uint16_t *)buf;
	const uint16_t * const edata = pdata + (sz>>1);

	if ( (uint16_t)s->wpage.pgm_address >= (PAGE_MAX * SPM_PAGESIZE) )
		return -ENOSPC;

	while ( pdata != edata )
	{
		// write a chunk of data till the end of page
		// or the end of actual data
		page_ofs_t offset = s->wpage.write_offset;
		while( offset < SPM_PAGESIZE && pdata != edata )
		{
			const unsigned bit_byte = offset
				/ sizeof(uint16_t)
				/ BITS_PER_BYTE
				;

			const unsigned bit_bit = offset
				/ sizeof(uint16_t)
				% BITS_PER_BYTE
				;

			s->wpage.header.bitmap[ bit_byte ] &= ~(1<<bit_bit);

			boot_page_fill(s->wpage.pgm_address + offset, *pdata++);
			offset += sizeof(uint16_t);
		}
		s->wpage.write_offset = offset;

		// check if we reached end of page
		// and if so, switch the page
		if ( SPM_PAGESIZE == offset )
		{
			uint16_t next = SPM_PAGESIZE
				+ ((uint16_t)s->wpage.pgm_address);

			__page_close(&s->wpage);

			if ( next >= PAGE_MAX*SPM_PAGESIZE )
				goto exit_wait; // ENOSPC

			__page_open(&s->wpage, next);
		}
	}

	__page_flush(&s->wpage);

exit_wait:
	boot_spm_busy_wait();
	return 0;
}

#ifdef SUPPORT_MULTISTREAM
flashdb_return_type flashdb_read(uint8_t stream, void *buf, size_t sz)
#else
flashdb_return_type flashdb_read(void *buf, size_t sz)
#endif
{
	((void)buf);
	((void)sz);
	return -ENXIO;
}

#ifdef SUPPORT_MULTISTREAM
flashdb_return_type flashdb_close(uint8_t stream)
#else
flashdb_return_type flashdb_close(void)
#endif
{
#ifndef SUPPORT_MULTISTREAM
	const uint8_t stream_id = 0;
#endif

	// TODO: sanity check
	struct stream * const s = streams + stream_id;

	if ( ((void*)(~0)) != s->wpage.pgm_address )
		__page_close(&s->wpage);

	s->flags &= ~SF_OPEN;

	return 0;
}

flashdb_return_type flashdb_erase(void)
{
#ifdef SUPPORT_MULTISTREAM
	const uint8_t stream_max = MAX_STREAMS;
#else
	const uint8_t stream_max = 1;
#endif
	for(uint8_t id = 0; id != stream_max; ++id)
		if ( streams[id].flags & SF_OPEN )
			return -EBUSY; // user has open streams

	for(uint16_t addr = 0;
	    addr != PAGE_MAX * SPM_PAGESIZE;
	    addr += SPM_PAGESIZE)
		__page_erase(addr);

	return 0;
}
