#include "gc/teraHeap/teraTransferPolicy.hpp"
#include "oops/oop.inline.hpp"

// TeraHeap in PS keeps in a bitmap all the objects that are
// candidates to be moved to H2. For this purpose in the
// implementations of h2_transfer_policy() for each different policy
// we do not examine the state of the object. The extra check is
// unnecessary.
// NOTE: Porting TeraHeap into a different collector you migh need to
// check if the object is candidate to be move to H2

bool DefaultPolicy::h2_transfer_policy(oop obj) {
    return true;
}

bool DefaultPolicy::h2_promotion_policy(oop obj) {
    return obj->is_marked_move_h2();
}

bool SparkPrimitivePolicy::h2_transfer_policy(oop obj) {
  return obj->is_primitive();
}

bool SparkPrimitivePolicy::h2_promotion_policy(oop obj) {
    return obj->is_marked_move_h2();
}

// Constructor
HintHighLowWatermarkPolicy::HintHighLowWatermarkPolicy() {
  direct_promotion = false;
  total_size_marked_obj_for_h2 = 0;
  h2_low_promotion_threshold = 0;
  non_promote_tag = 0;
  promote_tag = -1;
}

bool HintHighLowWatermarkPolicy::check_low_promotion_threshold(size_t sz) {
	if (h2_low_promotion_threshold == 0 || sz > h2_low_promotion_threshold)
		return false;

	h2_low_promotion_threshold -= sz;
	return true;
}

bool HintHighLowWatermarkPolicy::h2_transfer_policy(oop obj) {
  if (direct_promotion) {
    return check_low_promotion_threshold(obj->size());
  }

  return (obj->get_obj_group_id() <= promote_tag);
}

bool HintHighLowWatermarkPolicy::h2_promotion_policy(oop obj) {
  // We detect high memory presure in H1 heap and we are going to find
  // the transitive closure for all marked objects
  if (direct_promotion)
    return obj->is_marked_move_h2();

  return (obj->is_marked_move_h2() && obj->get_obj_group_id() <=  promote_tag);
}

bool HintHighLowWatermarkPrimitivePolicy::h2_transfer_policy(oop obj) {
  if (direct_promotion) {
    if (!obj->is_primitive())
      return false;

    return check_low_promotion_threshold(obj->size());
  }

  return (obj->is_primitive() && obj->get_obj_group_id() <= promote_tag);
}

