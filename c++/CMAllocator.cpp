/*
 *  COPYRIGHT (C) 2014 - 2020, piggy_xrh
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *	  Stpool is portable and efficient tasks pool library, it can works on diferent 
 * platforms such as Windows, linux, unix and ARM.  
 *
 *    If you have any troubles or questions on using the library, contact me.
 *
 * 	  (Email: piggy_xrh@163.com  QQ: 1169732280)
 * 	  blog: http://www.oschina.net/code/snippet_1987090_44422
 */
extern "C"
{
#include "ospx.h"
#include "mpool.h"
}
#include "CMAllocator.h"

CMAllocator::CMAllocator(const char *desc, size_t bytes):
	CAllocator(desc, bytes), m_ref(1) 
{
	mpool_init(&m_mp, bytes);
}

CMAllocator::~CMAllocator() 
{
	mpool_destroy(&m_mp, 1);
}

long CMAllocator::addRef()
{
	return OSPX_interlocked_increase(&m_ref);
}

long CMAllocator::release()
{
	long ref = OSPX_interlocked_decrease(&m_ref);

	if (!ref)
		delete this;

	return ref;
}

CAllocator *CMAllocator::clone(const char *desc) throw(std::bad_alloc)
{
	CAllocator *alc = new(std::nothrow) CMAllocator(desc, size());
	
	if (!alc)
		throw std::bad_alloc();
	return alc;
}

void *CMAllocator::alloc() throw()
{
	return mpool_new(&m_mp);
}

void  CMAllocator::dealloc(void *ptr) throw()
{
	mpool_delete(&m_mp, ptr);
}

void  CMAllocator::flush()
{
	mpool_flush(&m_mp);
}

CAllocator::Attr &CMAllocator::setAttr(Attr &attr) 
{
	struct mpool_attr_t at = {
		attr.nBlkSize, attr.nMinCache, attr.nMaxAlloc 
	};
	
	mpool_attr_set(&m_mp, &at);
	return getAttr(attr);
}

CAllocator::Attr &CMAllocator::getAttr(Attr &attr)
{
	struct mpool_attr_t at;
	
	mpool_attr_get(&m_mp, &at);
	
	attr.bReadOnly = false;
	attr.nBlkSize  = at.blk_size;
	attr.nMinCache = at.nmin_objs_cache;
	attr.nMaxAlloc = at.nmax_alloc;
	
	return attr;
}
		
CAllocator::Stat &CMAllocator::stat(CAllocator::Stat &s)
{
	struct mpool_stat_t st = {0};
	
	mpool_stat(&m_mp, &st);
	
	s.memHold = st.mem_hold_all;
	s.nCached = st.nobjs_resved;
	s.nAllocated = st.nobjs_allocated;
	s.nAcquired = st.nobjs_acquired;
	s.nBlks = st.nblks;
	
	return s;
}
