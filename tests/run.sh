#!/usr/bin/env bash

<<<<<<< HEAD
# Declare an associative array used for error handling
declare -A ERRORS

# Define the "error" values
ERRORS[INVALID_OPTION]=1
ERRORS[INVALID_ARG]=2
ERRORS[OUT_OF_RANGE]=3
ERRORS[NOT_AN_INTEGER]=4
ERRORS[PROGRAMMING_ERROR]=5

MODE=""
PARALLEL_GC_THREADS=()
STRIPE_SIZE=32768
jvm_build=""
cpu_arch=$(uname -p)
WRITE_POLICY="AsyncWritePolicy"
FLEXHEAP=false
FLEXHEAP_DEVICE="nvme0n1p1"
FLEXHEAP_MOUNT_POINT="/spare2/fmap/"
H2_ALLOCATOR_MODE=0 #0:Serial, 1:Parallel_H2PreCompact, 2:Paralell_H2Compact, 3:Parallel_H2PreCompact + Parallel_H2Compact
EXEC=("Array" 
	"Array_List" "Array_List_Int" 
	"List_Large" "MultiList"
  "Simple_Lambda" "Extend_Lambda" "Test_Reflection" "Test_Reference"
  "HashMap" "Rehashing" "Clone" 
  "Groupping"
  "MultiHashMap"
  "Test_WeakHashMap" "ClassInstance")
#EXEC=("Array")
# Export Enviroment Variables
export_env_vars() {
  PROJECT_DIR="$(pwd)/../.."
  export LIBRARY_PATH=${PROJECT_DIR}/allocator/lib/:$LIBRARY_PATH
  export LD_LIBRARY_PATH=${PROJECT_DIR}/allocator/lib/:$LD_LIBRARY_PATH
  export PATH=${PROJECT_DIR}/allocator/include/:$PATH
  export C_INCLUDE_PATH=${PROJECT_DIR}/allocator/include/:$C_INCLUDE_PATH
  export CPLUS_INCLUDE_PATH=${PROJECT_DIR}/allocator/include/:$CPLUS_INCLUDE_PATH

  export LIBRARY_PATH=${PROJECT_DIR}/tera_malloc/lib/:$LIBRARY_PATH
  export LD_LIBRARY_PATH=${PROJECT_DIR}/tera_malloc/lib/:$LD_LIBRARY_PATH
  export PATH=${PROJECT_DIR}/tera_malloc/include/:$PATH
  export C_INCLUDE_PATH=${PROJECT_DIR}/tera_malloc/include/:$C_INCLUDE_PATH
  export CPLUS_INCLUDE_PATH=${PROJECT_DIR}/tera_malloc/include/:$CPLUS_INCLUDE_PATH
}

clear_env() {
  echo "Clearing env..."
  local proj=$(pwd)
  
  echo "Clear H2 file..."
  cd /mnt/fmap
  rm -f h2-100.heap
  fallocate -l 100G h2-100.heap

  echo "Droping Caches..."
  sudo sync
  sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'

  echo "Done!"
  cd "$proj"
}

# Run tests using only interpreter mode
function interpreter_mode() {
  local class_file=$1
  local num_gc_thread=$2

  ${JAVA} -server \
    -XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly -XX:+PrintInterpreter -XX:+PrintNMethods \
    -Djava.compiler=NONE \
    -XX:+ShowMessageBoxOnError \
    -XX:+UseParallelGC \
    -XX:ParallelGCThreads=${num_gc_thread} \
    $(get_teraheap_flag) \
    -XX:TeraHeapSize=${TERACACHE_SIZE} \
    -Xmx${MAX}g \
    -Xms${XMS}g \
    -XX:-UseCompressedOops \
    -XX:-UseCompressedClassPointers \
    -XX:+TeraHeapStatistics \
    -XX:TeraStripeSize=${STRIPE_SIZE} \
    $(get_teraheap_mount_point) \
    $(get_flexheap_device) \
    $(get_h2_allocator_mode) \
    $(get_h2_write_policy) \
    -XX:H2FileSize=1288490188800 \
    -Xlogth:llarge_teraCache.txt "${class_file}" >err 2>&1 >out

}

# Run tests using only C1 compiler
function c1_mode() {
  local class_file=$1
  local num_gc_thread=$2

  ${JAVA} \
    -XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly \
    -XX:+PrintInterpreter \
    -XX:+PrintNMethods -XX:+PrintCompilation \
    -XX:+ShowMessageBoxOnError -XX:+LogCompilation \
    -XX:TieredStopAtLevel=3 -XX:+UseParallelGC \
    -XX:ParallelGCThreads=${num_gc_thread} \
    -XX:-UseParallelOldGC \
    $(get_teraheap_flag) \
    -XX:TeraHeapSize=${TERACACHE_SIZE} \
    -Xmx${MAX}g \
    -Xms${XMS}g \
    -XX:-UseCompressedOops \
    -XX:+TeraHeapStatistics \
    $(get_teraheap_mount_point) \
    $(get_flexheap_device) \
    $(get_h2_allocator_mode) \
    $(get_h2_write_policy) \
    -XX:H2FileSize=1288490188800 \
    -Xlogth:llarge_teraCache.txt "${class_file}" >err 2>&1 >out

}

# Run tests using C2 compiler
function c2_mode() {
  local class_file=$1
  local num_gc_thread=$2

  ${JAVA} \
    -server \
    -XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly \
    -XX:+PrintNMethods -XX:+PrintCompilation \
    -XX:+ShowMessageBoxOnError -XX:+LogCompilation \
    -XX:+UseParallelGC \
    -XX:ParallelGCThreads=${num_gc_thread} \
    -XX:-UseParallelOldGC \
    $(get_teraheap_flag) \
    -XX:TeraCacheSize=${TERACACHE_SIZE} \
    -Xmx${MAX}g \
    -Xms${XMS}g \
    -XX:TeraCacheThreshold=0 \
    -XX:-UseCompressedOops \
    -XX:+TeraCacheStatistics \
    $(get_teraheap_mount_point) \
    $(get_flexheap_device) \
    $(get_h2_allocator_mode) \
    $(get_h2_write_policy) \
    -XX:H2FileSize=1288490188800 \
    -Xlogtc:llarge_teraCache.txt "${class_file}" >err 2>&1 >run_tests.out
}

# Run tests using all compilers
function run_tests_msg_box() {
  local class_file=$1
  local num_gc_thread=$2

  ${JAVA} \
    -server \
    -XX:+ShowMessageBoxOnError \
    -XX:+UseParallelGC \
    -XX:ParallelGCThreads=${num_gc_thread} \
    $(get_teraheap_flag) \ 
  -XX:TeraHeapSize=${TERACACHE_SIZE} \
    -Xmx${MAX}g \
    -Xms${XMS}g \
    -XX:-UseCompressedOops \
    -XX:-UseCompressedClassPointers \
    -XX:+TeraHeapStatistics \
    -XX:TeraStripeSize=${STRIPE_SIZE} \
    $(get_teraheap_mount_point) \
    $(get_flexheap_device) \
    $(get_h2_allocator_mode) \
    $(get_h2_write_policy) \
    -XX:H2FileSize=1288490188800 \
    -Xlogth:llarge_teraCache.txt "${class_file}" >err 2>&1 >out
}

# Run tests using all compilers
function run_tests() {
  local class_file=$1
  local num_gc_thread=$2

  ${JAVA} \
    -XX:+UseParallelGC \
    -XX:+ShowMessageBoxOnError \
    -XX:ParallelGCThreads=${num_gc_thread} \
    $(get_teraheap_flag) \
    -XX:TeraHeapSize=${TERACACHE_SIZE} \
    -Xmx${MAX}g \
    -Xms${XMS}g \
    -XX:-UseCompressedOops \
    -XX:-UseCompressedClassPointers \
    -XX:+TeraHeapStatistics \
    -XX:TeraStripeSize=${STRIPE_SIZE} \
    $(get_teraheap_mount_point) \
    $(get_flexheap_device) \
    $(get_h2_allocator_mode) \
    $(get_h2_write_policy) \
    -XX:H2FileSize=1288490188800 \
    -Xlogth:llarge_teraCache.txt "${class_file}" >err 2>&1 >out
}

# Run tests using gdb
function run_tests_debug() {
  local class_file=$1
  local num_gc_thread=$2

  gdb --args ${JAVA} \
    -server \
    -XX:+ShowMessageBoxOnError \
    -XX:+UseParallelGC \
    -XX:ParallelGCThreads=${num_gc_thread} \
    $(get_teraheap_flag) \
    -XX:TeraHeapSize=${TERACACHE_SIZE} \
    -Xmx${MAX}g \
    -Xms${XMS}g \
    -XX:-UseCompressedOops \
    -XX:-UseCompressedClassPointers \
    -XX:+TeraHeapStatistics \
    -XX:TeraStripeSize=${STRIPE_SIZE} \
    $(get_teraheap_mount_point) \
    $(get_flexheap_device) \
    $(get_h2_allocator_mode) \
    $(get_h2_write_policy) \
    -XX:H2FileSize=1288490188800 \
    -Xlogth:llarge_teraCache.txt "${class_file}"
}

function get_h2_write_policy(){
  echo "-XX:TeraHeapWritePolicy=$WRITE_POLICY"
}

function get_flexheap_device() {
  if [ "$FLEXHEAP" == true ]; then
    echo "-XX:DEVICE_H2=$FLEXHEAP_DEVICE"
  else
    echo " "
  fi
}

function get_teraheap_mount_point() {
  echo "-XX:AllocateH2At=$FLEXHEAP_MOUNT_POINT"
}

function get_teraheap_flag() {
  echo "-XX:+EnableTeraHeap"
}

function get_h2_allocator_mode() {
  if [[ ! $H2_ALLOCATOR_MODE =~ ^[0-9]+$ ]]; then
    #echo "H2_ALLOCATOR_MODE:$H2_ALLOCATOR_MODE is not an integer."
    echo "Invalid H2_ALLOCATOR_MODE; Please provide 0:Serial, 1:Parallel_H2PreCompact, 2:Paralell_H2Compact, 3:Parallel_H2PreCompact + Parallel_H2Compact"
    exit ${ERRORS[NOT_AN_INTEGER]}
  elif [[ $H2_ALLOCATOR_MODE -lt 0 || $H2_ALLOCATOR_MODE -gt 3 ]]; then
    #echo "H2_ALLOCATOR_MODE:$H2_ALLOCATOR_MODE is not within the range 0 to 3."
    echo "Invalid H2_ALLOCATOR_MODE; Please provide 0:Serial, 1:Parallel_H2PreCompact, 2:Paralell_H2Compact, 3:Parallel_H2PreCompact + Parallel_H2Compact"
    exit ${ERRORS[OUT_OF_RANGE]}
  fi

  if [[ "$H2_ALLOCATOR_MODE" -eq 0 ]]; then
    echo "-XX:-EnableParallelH2PreCompact -XX:-EnableParallelH2Compact"
  elif [[ "$H2_ALLOCATOR_MODE" -eq 1 ]]; then
    echo "-XX:+EnableParallelH2PreCompact -XX:-EnableParallelH2Compact"
  elif [[ "$H2_ALLOCATOR_MODE" -eq 2 ]]; then
    echo "-XX:-EnableParallelH2PreCompact -XX:+EnableParallelH2Compact"
  elif [[ "$H2_ALLOCATOR_MODE" -eq 3 ]]; then
    echo "-XX:+EnableParallelH2PreCompact -XX:+EnableParallelH2Compact" 
  fi
}

# Usage
usage() {
  echo
  echo "Usage:"
  echo -n "      $0 [option ...] [-h]"
  echo
  echo "Options:"
  echo "      -p, --point    <mount_point>        The mount point used for the H2 file(eg. /mnt/fmap/)"
  echo "      -j, --jvm      <jvm_build>          The jvm build([release|r], [optimized|o], [fastdebug|f], Default: release)"
  echo "      -m, --mode     <execution_mode>     The jvm execution mode(0: Default, 1: Interpreter, 2: C1, 3: C2, 4: gdb, 5: ShowMessageBoxOnError)"
  echo "      -t, --threads  <threads>            The number of GC threads (2, 4, 8, 16, 32)"
  echo "      -w, --write-to-t2-policy <policy>   The available policies are: 'AsyncWritePolicy', 'SyncWritePolicy', 'FmapWritePolicy', 'DefaultWritePolicy'"
  echo "      -a, --h2-allocator <mode>           The available modes are: [0:Serial, 1:Parallel_H2PreCompact, 2:Paralell_H2Compact, 3:Parallel_H2PreCompact + Parallel_H2Compact]"
  echo "      -f, --flexheap                      Enable flexheap"
  echo "      -h  Show usage"
  echo

  exit 1
}

check_args() {
  # Check if required options are present
  if [[ -z "$MODE" || "${#PARALLEL_GC_THREADS[@]}" -eq 0 ]]; then
    echo "Usage: $0 -j <jvm_build> -m <mode> -t <#gcThreads,#gcThreads,#gcThreads> [-h]"
    echo "Example: ./run.sh -j release -m 0 -t 2 -r //execute a release jvm variant(-j release) using tiered compilation(-m 0) and 2 GC threads(-t 2)"
    echo "Example: ./run.sh -j fastdebug -m 0 -t 2 -d //execute a fastdebug jvm variant(-j fastdebug) using tiered compilation(-m 0) and 2 GC threads(-t 2)"
    exit 1
  fi
}

check_jvm_build() {
  # Validate jvm_build value
  if [[ "$jvm_build" != "" && "$jvm_build" != "fastdebug" && "$jvm_build" != "f" && "$jvm_build" != "optimized" && "$jvm_build" != "o" && "$jvm_build" != "release" && "$jvm_build" != "r" ]]; then
    echo "Error: jvm_build should be empty(for vanilla) or one of: [fastdebug|f], [optimized|o], [release|r]"
    usage
    exit 1
  fi
  # Use the appropriate java binary based on jvm_build
  if [[ "$jvm_build" == "fastdebug" || "$jvm_build" == "f" ]]; then
    JAVA="$(pwd)/../jdk17u067/build/linux-$cpu_arch-server-fastdebug/jdk/bin/java"
  elif [[ "$jvm_build" == "optimized" || "$jvm_build" == "o" || "$jvm_build" == "release" || "$jvm_build" == "r" ]]; then
    JAVA="$(pwd)/../jdk17u067/build/linux-$cpu_arch-server-release/jdk/bin/java"
  fi
}

check_h2_write_policy(){
  if [[ "$WRITE_POLICY" != "AsyncWritePolicy" && "$WRITE_POLICY" != "SyncWritePolicy" && "$WRITE_POLICY" != "FmapWritePolicy" && "$WRITE_POLICY" != "DefaultWritePolicy" ]]; then
    echo "Error: The available policies are 'AsyncWritePolicy', 'SyncWritePolicy', 'FmapWritePolicy', 'DefaultWritePolicy'"
    usage
    exit 1
  fi
}

print_msg() {
  local gcThread=$1
  local mode_value
  case "${MODE}" in
  0)
    mode_value="Default"
    ;;
  1)
    mode_value="Interpreter"
    ;;
  2)
    mode_value="C1"
    ;;
  3)
    mode_value="C2"
    ;;
  esac

  echo "___________________________________"
  echo
  echo "         Run JAVA Tests"
  echo
  echo "Mode:       ${mode_value}"
  echo "GC Threads: ${gcThread}"
  echo "___________________________________"
  echo
}

function parse_script_arguments() {
  local OPTIONS=p:j:m:t:w:a:fh
  local LONGOPTIONS=point:,jvm:,mode:,threads:,write-to-h2-policy:,h2-allocator:,flexheap,help

  # Use getopt to parse the options
  local PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTIONS --name "$0" -- "$@")

  # Check for errors in getopt
  if [[ $? -ne 0 ]]; then
    return ${ERRORS[INVALID_OPTION]} 2>/dev/null || exit ${ERRORS[INVALID_OPTION]}
  fi

  # Evaluate the parsed options
  eval set -- "$PARSED"

  while true; do
    case "$1" in
    -p | --point)
      FLEXHEAP_MOUNT_POINT="$2"
      # Find the device name using df and process it to remove the /dev/ prefix
      FLEXHEAP_DEVICE=$(df "$FLEXHEAP_MOUNT_POINT" | awk 'NR==2 {print $1}' | sed 's|/dev/||')
      shift 2
      ;;
    -j | --jvm)
      jvm_build="$2"
      shift 2
      ;;
    -m | --mode)
      MODE="$2"
      shift 2
      ;;
    -t | --threads)
      IFS=',' read -r -a PARALLEL_GC_THREADS <<<"$2"
      shift 2
      ;;
    -w | --write-to-h2-policy)
     WRITE_POLICY="$2"
     shift 2
     ;;
    -a | --h2-allocator)
      H2_ALLOCATOR_MODE="$2"
      shift 2
      ;;
    -f | --flexheap)
      FLEXHEAP=true
      shift
      ;; 
    -h | --help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    *)
      echo "Programming error"
      exit ${ERRORS[PROGRAMMING_ERROR]}
      ;;
    esac
  done
}

parse_script_arguments "$@"
check_args
check_h2_write_policy
check_jvm_build

cd java || exit

for gcThread in "${PARALLEL_GC_THREADS[@]}"; do
  print_msg "$gcThread"

  for exec_file in "${EXEC[@]}"; do
    if [ "${exec_file}" == "ClassInstance" ]; then
      XMS=2
    elif [ "${exec_file}" == "Array_List" ]; then
      XMS=64
    else
      XMS=1
    fi

    MAX=1200
    TERACACHE_SIZE=$(echo $(((MAX - XMS) * 1024 * 1024 * 1024)))
    case ${MODE} in
    0)
      export_env_vars
      run_tests "$exec_file" "$gcThread"
      ;;
    1)
      export_env_vars
      interpreter_mode "$exec_file" "$gcThread"
      ;;
    2)
      export_env_vars
      c1_mode "$exec_file" "$gcThread"
      ;;
    3)
      export_env_vars
      c2_mode "$exec_file" "$gcThread"
      ;;
    4)
      export_env_vars
      run_tests_debug "$exec_file" "$gcThread"
      ;;
    5)
      export_env_vars
      run_tests_msg_box "$exec_file" "$gcThread"
      ;;
    esac

    ans=$?

    echo -ne "${exec_file} "

    if [ $ans -eq 0 ]; then
      echo -e '\e[30G \e[32;1mPASS\e[0m'
    else
      echo -e '\e[30G \e[31;1mFAIL\e[0m'
      break
    fi
  done

done

cd - >/dev/null || exit

