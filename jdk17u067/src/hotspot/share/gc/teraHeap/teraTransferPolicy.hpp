#ifndef SHARE_GC_TERAHEAP_TERATRANSFERPOLICY_HPP
#define SHARE_GC_TERAHEAP_TERATRANSFERPOLICY_HPP

#include "memory/allocation.hpp"
#include "oops/oopsHierarchy.hpp"
#include "gc/shared/gc_globals.hpp"

#define LOW_THRESHOLD_WEIGHT 0.5f
#define HIGH_THRESHOLD 0.85

class TransferPolicy : public CHeapObj<mtInternal> {
protected:
  bool transfer_on;

public:
  // Set non promote label value
  virtual void set_non_promote_tag(long val) = 0;
  // Set promote tag value
  virtual void set_promote_tag(long val) = 0;
  // Get non promote tag value
  virtual long get_non_promote_tag() = 0;
  // Get promote tag value
  virtual long get_promote_tag() = 0;

  virtual void set_direct_promotion(size_t old_live, size_t max_old_gen_size) = 0;
  virtual void unset_direct_promotion() = 0;
  virtual void set_low_promotion_threshold() = 0;
  virtual void h2_incr_total_marked_obj_size(size_t sz) = 0;
  virtual void h2_reset_total_marked_obj_size() = 0;

  // This function determines which of the H2 candidate objects found
  // during marking phase we are going to move to H2. According to the
  // policy that we enabled in the sharedDefines.h file we do the
  // appropriate action. This function is used only in the
  // precompaction phase.
  virtual bool h2_transfer_policy(oop obj) = 0;

  // Promotion policy for H2 candidate objects. This function is used
  // during the marking phase of the major GC. According to the policy
  // that we enabled in the sharedDefines.h file we do the appropriate
  // action.
  virtual bool h2_promotion_policy(oop obj) = 0;

  void set_move_h2() { transfer_on = true;}
  void unset_move_h2() { transfer_on = false;}
  bool is_tranfer_on() { return transfer_on; }
};

class DefaultPolicy : public TransferPolicy {
public:
  DefaultPolicy() { transfer_on = true; }
  void set_non_promote_tag(long val) override {}
  void set_promote_tag(long val) override {}
  long get_non_promote_tag() override { return 0; }
  long get_promote_tag() override { return 0; }

  void set_direct_promotion(size_t old_live, size_t max_old_gen_size) override {}
  void unset_direct_promotion() override {};
  void set_low_promotion_threshold() override {}
  void h2_incr_total_marked_obj_size(size_t sz) override {}
  void h2_reset_total_marked_obj_size() override {}

  bool h2_transfer_policy(oop obj) override;
  bool h2_promotion_policy(oop obj) override;
};

class SparkPrimitivePolicy : public TransferPolicy {
public:
  SparkPrimitivePolicy() { transfer_on = true; }
  void set_non_promote_tag(long val) override {}
  void set_promote_tag(long val) override {}
  long get_non_promote_tag() override { return 0; }
  long get_promote_tag() override { return 0; }

  void set_direct_promotion(size_t old_live, size_t max_old_gen_size) override {}
  void unset_direct_promotion() override {};
  void set_low_promotion_threshold() override {}
  void h2_incr_total_marked_obj_size(size_t sz) override {}
  void h2_reset_total_marked_obj_size() override {}

  bool h2_transfer_policy(oop obj) override;
  bool h2_promotion_policy(oop obj) override;
};

class HintHighLowWatermarkPolicy : public TransferPolicy {
protected:
  bool direct_promotion;                  //< Direct promotion threshold
  size_t total_size_marked_obj_for_h2;    //< Total size of H2
                                          //candidate objects found in marking phase of major GC
  size_t h2_low_promotion_threshold;      //< Low promotion threshold
  long non_promote_tag;                   //< Object with this label
                                          //cannot be promoted to H2
  long promote_tag;                       //< Objects with labels less than
                                          // the promote_tag can be moved to
                                          // H2 during major GC

public:
  HintHighLowWatermarkPolicy();

  void set_non_promote_tag(long val) override { non_promote_tag = 0;    }
  void set_promote_tag(long val) override     { promote_tag = val;      }
  long get_non_promote_tag() override         { return non_promote_tag; }
  long get_promote_tag() override             { return promote_tag;     }

  void set_direct_promotion(size_t old_live, size_t max_old_gen_size) override {
    direct_promotion = (DynamicHeapResizing) ? true :
      ((float) old_live / (float) max_old_gen_size) >= HIGH_THRESHOLD ? true : false;
  }
  
  void unset_direct_promotion() override {
    direct_promotion = false;
  };

  void set_low_promotion_threshold() override {
    h2_low_promotion_threshold = 
      (size_t) (total_size_marked_obj_for_h2 * LOW_THRESHOLD_WEIGHT);
  }

  void h2_incr_total_marked_obj_size(size_t sz) override {
    total_size_marked_obj_for_h2 += sz;
  }

  void h2_reset_total_marked_obj_size() override {
    total_size_marked_obj_for_h2 = 0;
  }

  bool check_low_promotion_threshold(size_t sz);

  bool h2_transfer_policy(oop obj) override;
  bool h2_promotion_policy(oop obj) override;
};

class HintHighLowWatermarkPrimitivePolicy : public HintHighLowWatermarkPolicy {
public:
  HintHighLowWatermarkPrimitivePolicy() { transfer_on = false; }
  bool h2_transfer_policy(oop obj) override;
};

#endif // SHARE_GC_TERAHEAP_TERATRANSFERPOLICY_HPP
