#ifndef SHARE_VM_GC_IMPLEMENTATION_TERAHEAP_TERAENUMS_HPP
#define SHARE_VM_GC_IMPLEMENTATION_TERAHEAP_TERAENUMS_HPP

enum actions {
  NO_ACTION,                      //< Do not perform any action
  SHRINK_H1,                      //< Shrink H1 because the I/O is high
  GROW_H1,                        //< Grow H1 because the GC is high
  MOVE_BACK,                      //< Move obects from H2 to H1 
  CONTINUE,                       //< Continue not finished interval
  MOVE_H2,                        //< Transfer objects to H2
  IOSLACK,                        //< IO slack for page cache to grow
  WAIT_AFTER_GROW                 //< After grow wait to see the effect
};

enum states {
  S_NO_ACTION,                     //< No action state
  S_WAIT_SHRINK,                   //< After shrinking h1 wait for the effect
  S_WAIT_GROW,                     //< After growing h1 wait for the effect
  S_WAIT_MOVE                      //< Wait for moving objects to H2
};

#endif
