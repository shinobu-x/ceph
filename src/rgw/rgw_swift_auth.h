// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef CEPH_RGW_SWIFT_AUTH_H
#define CEPH_RGW_SWIFT_AUTH_H

#include "rgw_op.h"
#include "rgw_rest.h"
#include "rgw_auth.h"

#define RGW_SWIFT_TOKEN_EXPIRATION (15 * 60)

/* TempURL: applier. */
class RGWTempURLAuthApplier : public RGWLocalAuthApplier {
public:
  RGWTempURLAuthApplier(CephContext * const cct,
                        const RGWUserInfo& user_info)
    : RGWLocalAuthApplier(cct, user_info, RGWLocalAuthApplier::NO_SUBUSER) {
  };

  void modify_request_state(req_state * s) const override; /* in/out */

  struct Factory {
    virtual ~Factory() {}
    virtual aplptr_t create_apl_turl(CephContext * const cct,
                                     const RGWUserInfo& user_info) const = 0;
  };
};

/* TempURL: engine */
class RGWTempURLAuthEngine : public RGWAuthEngine {
protected:
  /* const */ RGWRados * const store;
  const req_state * const s;
  const RGWTempURLAuthApplier::Factory * const apl_factory;

  /* Helper methods. */
  void get_owner_info(RGWUserInfo& owner_info) const;
  bool is_expired(const std::string& expires) const;

  class SignatureHelper;

public:
  RGWTempURLAuthEngine(const req_state * const s,
                       /*const*/ RGWRados * const store,
                       const RGWTempURLAuthApplier::Factory * const apl_factory)
    : RGWAuthEngine(s->cct),
      store(store),
      s(s),
      apl_factory(apl_factory) {
  }

  /* Interface implementations. */
  const char* get_name() const noexcept override {
    return "RGWTempURLAuthEngine";
  }

  bool is_applicable() const noexcept override;
  RGWAuthApplier::aplptr_t authenticate() const override;
};


/* AUTH_rgwtk */
class RGWSignedTokenAuthEngine : public RGWTokenBasedAuthEngine {
protected:
  /* const */ RGWRados * const store;
  const RGWLocalAuthApplier::Factory * apl_factory;
public:
  RGWSignedTokenAuthEngine(CephContext * const cct,
                           /* const */RGWRados * const store,
                           const Extractor& extr,
                           const RGWLocalAuthApplier::Factory * const apl_factory)
    : RGWTokenBasedAuthEngine(cct, extr),
      store(store),
      apl_factory(apl_factory) {
  }

  const char* get_name() const noexcept override {
    return "RGWSignedTokenAuthEngine";
  }

  bool is_applicable() const noexcept override;
  RGWAuthApplier::aplptr_t authenticate() const override;
};


/* External token */
class RGWExternalTokenAuthEngine : public RGWTokenBasedAuthEngine {
protected:
  /* const */ RGWRados * const store;
  const RGWLocalAuthApplier::Factory * const apl_factory;
public:
  RGWExternalTokenAuthEngine(CephContext * const cct,
                             /* const */RGWRados * const store,
                             const Extractor& extr,
                             const RGWLocalAuthApplier::Factory * const apl_factory)
    : RGWTokenBasedAuthEngine(cct, extr),
      store(store),
      apl_factory(apl_factory) {
  }

  const char* get_name() const noexcept override {
    return "RGWExternalTokenAuthEngine";
  }

  bool is_applicable() const noexcept override;
  RGWAuthApplier::aplptr_t authenticate() const override;
};


/* Extractor for X-Auth-Token present in req_state. */
class RGWXAuthTokenExtractor : public RGWTokenBasedAuthEngine::Extractor {
protected:
  const req_state * const s;
public:
  RGWXAuthTokenExtractor(const req_state * const s)
    : s(s) {
  }
  std::string get_token() const override {
    /* Returning a reference here would end in GCC complaining about a reference
     * to temporary. */
    return s->info.env->get("HTTP_X_AUTH_TOKEN", "");
  }
};


class RGW_SWIFT_Auth_Get : public RGWOp {
public:
  RGW_SWIFT_Auth_Get() {}
  ~RGW_SWIFT_Auth_Get() override {}

  int verify_permission() override { return 0; }
  void execute() override;
  const string name() override { return "swift_auth_get"; }
};

class RGWHandler_SWIFT_Auth : public RGWHandler_REST {
public:
  RGWHandler_SWIFT_Auth() {}
  ~RGWHandler_SWIFT_Auth() override {}
  RGWOp *op_get() override;

  int init(RGWRados *store, struct req_state *state, rgw::io::BasicClient *cio) override;
  int authorize() override;
  int postauth_init() override { return 0; }
  int read_permissions(RGWOp *op) override { return 0; }

  virtual RGWAccessControlPolicy *alloc_policy() { return NULL; }
  virtual void free_policy(RGWAccessControlPolicy *policy) {}
};

class RGWRESTMgr_SWIFT_Auth : public RGWRESTMgr {
public:
  RGWRESTMgr_SWIFT_Auth() = default;
  ~RGWRESTMgr_SWIFT_Auth() override = default;

  RGWRESTMgr *get_resource_mgr(struct req_state* const s,
                               const std::string& uri,
                               std::string* const out_uri) override {
    return this;
  }

  RGWHandler_REST* get_handler(struct req_state*,
                               const std::string&) override {
    return new RGWHandler_SWIFT_Auth;
  }
};


#endif
