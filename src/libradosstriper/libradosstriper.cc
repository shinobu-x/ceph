// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2014 Sebastien Ponce <sebastien.ponce@cern.ch>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include <errno.h>

#include "libradosstriper/RadosStriperImpl.h"
#include "libradosstriper/MultiAioCompletionImpl.h"

#include "include/types.h"

#include "include/radosstriper/libradosstriper.h"
#include "include/radosstriper/libradosstriper.hpp"
#include "librados/RadosXattrIter.h"

using namespace libradosstriper;

/*
 * This file implements the rados striper API.
 * There are 2 flavours of it :
 *   - the C API, found in include/rados/libradosstriper.h
 *   - the C++ API, found in include/rados/libradosstriper.hpp
 */

///////////////////////////// C++ API //////////////////////////////

MultiAioCompletion::~MultiAioCompletion()
{
  delete pc;
}

int MultiAioCompletion::set_complete_callback(
  void *cb_arg, 
  rados_callback_t cb)
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  return c->set_complete_callback(cb_arg, cb);
}

int MultiAioCompletion::set_safe_callback(
  void *cb_arg,
  rados_callback_t cb)
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  return c->set_safe_callback(cb_arg, cb);
}

void MultiAioCompletion::wait_for_complete()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  c->wait_for_complete();
}

void MultiAioCompletion::wait_for_safe()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  c->wait_for_safe();
}

bool MultiAioCompletion::is_complete()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  return c->is_complete();
}

bool MultiAioCompletion::is_safe()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  return c->is_safe();
}

void MultiAioCompletion::wait_for_complete_and_cb()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  c->wait_for_complete_and_cb();
}

void MultiAioCompletion::MultiAioCompletion::wait_for_safe_and_cb()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  c->wait_for_safe_and_cb();
}

bool MultiAioCompletion::is_complete_and_cb()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  return c->is_complete_and_cb();
}

bool MultiAioCompletion::is_safe_and_cb()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  return c->is_safe_and_cb();
}

int MultiAioCompletion::get_return_value()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  return c->get_return_value();
}

void MultiAioCompletion::release()
{
  MultiAioCompletionImpl *c = (MultiAioCompletionImpl *)pc;
  c->put();
  delete this;
}

RadosStriper::RadosStriper() :
  rados_striper_impl(0)
{
}

void RadosStriper::to_rados_striper_t(
  RadosStriper &striper,
  rados_striper_t *s)
{
  *s = (rados_striper_t)striper.rados_striper_impl;
  striper.rados_striper_impl->get();
}

RadosStriper::RadosStriper(
  const RadosStriper& rs)
{
  rados_striper_impl = rs.rados_striper_impl;
  if (rados_striper_impl) {
    rados_striper_impl->get();
  }
}

RadosStriper& RadosStriper::operator=(
  const RadosStriper& rs)
{
  if (rados_striper_impl)
    rados_striper_impl->put();
  rados_striper_impl = rs.rados_striper_impl;
  rados_striper_impl->get();
  return *this;
}

RadosStriper::~RadosStriper()
{
  if (rados_striper_impl)
    rados_striper_impl->put();
  rados_striper_impl = 0;
}

int RadosStriper::striper_create(
  librados::IoCtx& ioctx,
  RadosStriper *striper)
{
  try {
    striper->rados_striper_impl =
      new RadosStriperImpl(ioctx, ioctx.io_ctx_impl);
    striper->rados_striper_impl->get();
  } catch (int rc) {
    return rc;
  }
  return 0;
}

int RadosStriper::set_object_layout_stripe_unit(
  unsigned int stripe_unit)
{
  return rados_striper_impl->setObjectLayoutStripeUnit(stripe_unit);
}

int RadosStriper::set_object_layout_stripe_count(
  unsigned int stripe_count)
{
  return rados_striper_impl->setObjectLayoutStripeCount(stripe_count);
}

int RadosStriper::set_object_layout_object_size(
  unsigned int object_size)
{
  return rados_striper_impl->setObjectLayoutObjectSize(object_size);
}

int RadosStriper::getxattr(
  const std::string& oid,
  const char *name,
  bufferlist& bl)
{
  return rados_striper_impl->getxattr(oid, name, bl);
}

int RadosStriper::setxattr(
  const std::string& oid, 
  const char *name,
  bufferlist& bl)
{
  return rados_striper_impl->setxattr(oid, name, bl);
}

int RadosStriper::rmxattr(
  const std::string& oid,
  const char *name)
{
  return rados_striper_impl->rmxattr(oid, name);
}

int RadosStriper::getxattrs(
 const std::string& oid,
 std::map<std::string, bufferlist>& attrset)
{
  return rados_striper_impl->getxattrs(oid, attrset);
}

int RadosStriper::write(
  const std::string& soid,
  const bufferlist& bl,
  size_t len,
  uint64_t off)
{
  return rados_striper_impl->write(soid, bl, len, off);
}

int RadosStriper::write_full(
  const std::string& soid,
  const bufferlist& bl)
{
  return rados_striper_impl->write_full(soid, bl);
}

int RadosStriper::append(
  const std::string& soid,
  const bufferlist& bl,
  size_t len)
{
  return rados_striper_impl->append(soid, bl, len);
}

int RadosStriper::aio_write(
  const std::string& soid,
  librados::AioCompletion *c,
  const bufferlist& bl,
  size_t len,
  uint64_t off)
{
  return rados_striper_impl->aio_write(soid, c->pc, bl, len, off);
}

int RadosStriper::aio_write_full(
  const std::string& soid,
  librados::AioCompletion *c,
  const bufferlist& bl)
{
  return rados_striper_impl->aio_write_full(soid, c->pc, bl);
}

int RadosStriper::aio_append(
  const std::string& soid,
  librados::AioCompletion *c,
  const bufferlist& bl,
  size_t len)
{
  return rados_striper_impl->aio_append(soid, c->pc, bl, len);
}

int RadosStriper::read(
  const std::string& soid,
  bufferlist* bl,
  size_t len,
  uint64_t off)
{
  bl->clear();
  bl->push_back(buffer::create(len));
  return rados_striper_impl->read(soid, bl, len, off);
}

int RadosStriper::aio_read(
  const std::string& soid,
  librados::AioCompletion *c,
  bufferlist* bl,
  size_t len,
  uint64_t off)
{
  bl->clear();
  bl->push_back(buffer::create(len));
  return rados_striper_impl->aio_read(soid, c->pc, bl, len, off);
}

int RadosStriper::stat(
  const std::string& soid,
  uint64_t *psize,
  time_t *pmtime)
{
  return rados_striper_impl->stat(soid, psize, pmtime);
}

int RadosStriper::aio_stat(
  const std::string& soid,
  librados::AioCompletion *c,
  uint64_t *psize,
  time_t *pmtime)
{
  return rados_striper_impl->aio_stat(soid, c->pc, psize, pmtime);
}

int RadosStriper::stat2(
  const std::string& soid,
  uint64_t *psize,
  struct timespec *pts)
{
  return rados_striper_impl->stat2(soid, psize, pts);
}

int RadosStriper::aio_stat2(
  const std::string& soid,
  librados::AioCompletion *c,
  uint64_t *psize,
  struct timespec *pts)
{
  return rados_striper_impl->aio_stat2(soid, c->pc, psize, pts);
}

int RadosStriper::remove(
  const std::string& soid)
{
  return rados_striper_impl->remove(soid);
}

int RadosStriper::aio_remove(
  const std::string& soid,
  librados::AioCompletion *c)
{
  return rados_striper_impl->aio_remove(soid, c->pc);
}

int RadosStriper::remove(
  const std::string& soid,
  int flags)
{
  return rados_striper_impl->remove(soid, flags); 
}

int RadosStriper::aio_remove(
  const std::string& soid,
  librados::AioCompletion *c,
  int flags)
{
  return rados_striper_impl->aio_remove(soid, c->pc, flags);
}

int RadosStriper::trunc(
  const std::string& soid,
  uint64_t size)
{
  return rados_striper_impl->trunc(soid, size);
}

int RadosStriper::aio_flush()
{
  return rados_striper_impl->aio_flush();
}

MultiAioCompletion* RadosStriper::multi_aio_create_completion()
{
  MultiAioCompletionImpl *c = new MultiAioCompletionImpl;
  return new MultiAioCompletion(c);
}

MultiAioCompletion* RadosStriper::multi_aio_create_completion(
  void *cb_arg,
  librados::callback_t cb_complete,
  librados::callback_t cb_safe)
{
  MultiAioCompletionImpl *c;
  int r = rados_striper_multi_aio_create_completion(
    cb_arg, cb_complete, cb_safe, (void**)&c);

  assert(r == 0);

  return new MultiAioCompletion(c);
}

///////////////////////////// C API //////////////////////////////

extern "C" int rados_striper_create(
  rados_ioctx_t ioctx,
  rados_striper_t *striper)
{
  librados::IoCtx ctx;
  librados::IoCtx::from_rados_ioctx_t(ioctx, ctx);
  RadosStriper striperp;
  int rc = RadosStriper::striper_create(ctx, &striperp);
  if (0 == rc)
    RadosStriper::to_rados_striper_t(striperp, striper);
  return rc;
}

extern "C" void rados_striper_destroy(
  rados_striper_t striper)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  impl->put();
}

extern "C" int rados_striper_set_object_layout_stripe_unit(
  rados_striper_t striper,
  unsigned int stripe_unit)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  return impl->setObjectLayoutStripeUnit(stripe_unit);
}

extern "C" int rados_striper_set_object_layout_stripe_count(
  rados_striper_t striper,
  unsigned int stripe_count)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  return impl->setObjectLayoutStripeCount(stripe_count);
}

extern "C" int rados_striper_set_object_layout_object_size(
  rados_striper_t striper,
  unsigned int object_size)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  return impl->setObjectLayoutObjectSize(object_size);
}

extern "C" int rados_striper_write(
  rados_striper_t striper,
  const char *soid,
  const char *buf,
  size_t len,
  uint64_t off)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  bufferlist bl;
  bl.append(buf, len);
  return impl->write(soid, bl, len, off);
}

extern "C" int rados_striper_write_full(
  rados_striper_t striper,
  const char *soid,
  const char *buf,
  size_t len)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  bufferlist bl;
  bl.append(buf, len);
  return impl->write_full(soid, bl);
}


extern "C" int rados_striper_append(
  rados_striper_t striper,
  const char *soid,
  const char *buf,
  size_t len)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  bufferlist bl;
  bl.append(buf, len);
  return impl->append(soid, bl, len);
}

extern "C" int rados_striper_read(
  rados_striper_t striper,
  const char *soid,
  char *buf,
  size_t len,
  uint64_t off)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  bufferlist bl;
  bufferptr bp = buffer::create_static(len, buf);
  bl.push_back(bp);
  int ret = impl->read(soid, &bl, len, off);
  if (ret >= 0) {
    if (bl.length() > len)
      return -ERANGE;
    if (!bl.is_provided_buffer(buf))
      bl.copy(0, bl.length(), buf);
    ret = bl.length();    // hrm :/
  }
  return ret;
}

extern "C" int rados_striper_remove(
  rados_striper_t striper,
  const char* soid)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  return impl->remove(soid);
}

extern "C" int rados_striper_trunc(
  rados_striper_t striper, 
  const char* soid,
  uint64_t size)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  return impl->trunc(soid, size);
}

extern "C" int rados_striper_getxattr(
  rados_striper_t striper,
  const char *oid,
  const char *name,
  char *buf,
  size_t len)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  object_t obj(oid);
  bufferlist bl;
  int ret = impl->getxattr(oid, name, bl);
  if (ret >= 0) {
    if (bl.length() > len)
      return -ERANGE;
    bl.copy(0, bl.length(), buf);
    ret = bl.length();
  }
  return ret;
}

extern "C" int rados_striper_setxattr(
  rados_striper_t striper,
  const char *oid,
  const char *name,
  const char *buf,
  size_t len)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  object_t obj(oid);
  bufferlist bl;
  bl.append(buf, len);
  return impl->setxattr(obj, name, bl);
}

extern "C" int rados_striper_rmxattr(
  rados_striper_t striper,
  const char *oid,
  const char *name)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  object_t obj(oid);
  return impl->rmxattr(obj, name);
}

extern "C" int rados_striper_getxattrs(
  rados_striper_t striper,
  const char *oid,
  rados_xattrs_iter_t *iter)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  object_t obj(oid);
  librados::RadosXattrsIter *it = new librados::RadosXattrsIter();
  if (!it)
    return -ENOMEM;
  int ret = impl->getxattrs(obj, it->attrset);
  if (ret) {
    delete it;
    return ret;
  }
  it->i = it->attrset.begin();
  librados::RadosXattrsIter **iret = (librados::RadosXattrsIter**)iter;
  *iret = it;
  *iter = it;
  return 0;
}

extern "C" int rados_striper_getxattrs_next(
  rados_xattrs_iter_t iter,
  const char **name,
  const char **val,
  size_t *len)
{
  return rados_getxattrs_next(iter, name, val, len);
}

extern "C" void rados_striper_getxattrs_end(
  rados_xattrs_iter_t iter)
{
  return rados_getxattrs_end(iter);
}

extern "C" int rados_striper_stat(
  rados_striper_t striper,
  const char* soid,
  uint64_t *psize,
  time_t *pmtime)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  return impl->stat(soid, psize, pmtime);
}

extern "C" int rados_striper_multi_aio_create_completion(
  void *cb_arg,
  rados_callback_t cb_complete,
  rados_callback_t cb_safe,
  rados_striper_multi_completion_t *pc)
{
  MultiAioCompletionImpl *c = new MultiAioCompletionImpl;
  if (cb_complete)
    c->set_complete_callback(cb_arg, cb_complete);
  if (cb_safe)
    c->set_safe_callback(cb_arg, cb_safe);
  *pc = c;
  return 0;
}

extern "C" void rados_striper_multi_aio_wait_for_complete(
  rados_striper_multi_completion_t c)
{
  ((MultiAioCompletionImpl*)c)->wait_for_complete();
}

extern "C" void rados_striper_multi_aio_wait_for_safe(
  rados_striper_multi_completion_t c)
{
  ((MultiAioCompletionImpl*)c)->wait_for_safe();
}

extern "C" int rados_striper_multi_aio_is_complete(
  rados_striper_multi_completion_t c)
{
  return ((MultiAioCompletionImpl*)c)->is_complete();
}

extern "C" int rados_striper_multi_aio_is_safe(
  rados_striper_multi_completion_t c)
{
  return ((MultiAioCompletionImpl*)c)->is_safe();
}

extern "C" void rados_striper_multi_aio_wait_for_complete_and_cb(
  rados_striper_multi_completion_t c)
{
  ((MultiAioCompletionImpl*)c)->wait_for_complete_and_cb();
}

extern "C" void rados_striper_multi_aio_wait_for_safe_and_cb(
  rados_striper_multi_completion_t c)
{
  ((MultiAioCompletionImpl*)c)->wait_for_safe_and_cb();
}

extern "C" int rados_striper_multi_aio_is_complete_and_cb(
  rados_striper_multi_completion_t c)
{
  return ((MultiAioCompletionImpl*)c)->is_complete_and_cb();
}

extern "C" int rados_striper_multi_aio_is_safe_and_cb(
  rados_striper_multi_completion_t c)
{
  return ((MultiAioCompletionImpl*)c)->is_safe_and_cb();
}

extern "C" int rados_striper_multi_aio_get_return_value(
  rados_striper_multi_completion_t c)
{
  return ((MultiAioCompletionImpl*)c)->get_return_value();
}

extern "C" void rados_striper_multi_aio_release(
  rados_striper_multi_completion_t c)
{
  ((MultiAioCompletionImpl*)c)->put();
}

extern "C" int rados_striper_aio_write(
  rados_striper_t striper,
  const char* soid,
  rados_completion_t completion,
  const char *buf,
  size_t len,
  uint64_t off)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  bufferlist bl;
  bl.append(buf, len);
  return impl->aio_write(soid, (librados::AioCompletionImpl*)completion, bl, len, off);
}

extern "C" int rados_striper_aio_append(
  rados_striper_t striper,
  const char* soid,
  rados_completion_t completion,
  const char *buf,
  size_t len)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  bufferlist bl;
  bl.append(buf, len);
  return impl->aio_append(soid, (
    librados::AioCompletionImpl*)completion, bl, len);
}

extern "C" int rados_striper_aio_write_full(
  rados_striper_t striper,
  const char* soid,
  rados_completion_t completion,
  const char *buf,
  size_t len)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  bufferlist bl;
  bl.append(buf, len);
  return impl->aio_write_full(
    soid, (librados::AioCompletionImpl*)completion, bl);
}

extern "C" int rados_striper_aio_read(
  rados_striper_t striper,
  const char *soid,
  rados_completion_t completion,
  char *buf,
  size_t len,
  uint64_t off)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  return impl->aio_read(
    soid, (librados::AioCompletionImpl*)completion, buf, len, off);
}

extern "C" int rados_striper_aio_remove(
  rados_striper_t striper,
  const char* soid,
  rados_completion_t completion)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  return impl->aio_remove(soid, (librados::AioCompletionImpl*)completion);
}

extern "C" void rados_striper_aio_flush(
  rados_striper_t striper)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  impl->aio_flush();
}

extern "C" int rados_striper_aio_stat(
  rados_striper_t striper,
  const char* soid,
  rados_completion_t completion,
  uint64_t *psize,
  time_t *pmtime)
{
  RadosStriperImpl *impl = (RadosStriperImpl *)striper;
  return impl->aio_stat(soid, (
    librados::AioCompletionImpl*)completion, psize, pmtime);
}
