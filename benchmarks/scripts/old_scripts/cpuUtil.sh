LOG_START=$1        # CPU Utilization before run
LOG_END=$2          # CPU Utilization after run

USR1=`awk -F " " '{print $2}' $LOG_START`
NICE1=`awk -F " " '{print $3}' $LOG_START`
SYS1=`awk -F " " '{print $4}' $LOG_START`
IDLE1=`awk -F " " '{print $5}' $LOG_START`
IOWAIT1=`awk -F " " '{print $6}' $LOG_START`
IRQ1=`awk -F " " '{print $7}' $LOG_START`
SOFRIRQ1=`awk -F " " '{print $8}' $LOG_START`
STEAL1=`awk -F " " '{print $9}' $LOG_START`
GUEST1=`awk -F " " '{print $10}' $LOG_START`

USR2=`awk -F " " '{print $2}' $LOG_END`
NICE2=`awk -F " " '{print $3}' $LOG_END`
SYS2=`awk -F " " '{print $4}' $LOG_END`
IDLE2=`awk -F " " '{print $5}' $LOG_END`
IOWAIT2=`awk -F " " '{print $6}' $LOG_END`
IRQ2=`awk -F " " '{print $7}' $LOG_END`
SOFRIRQ2=`awk -F " " '{print $8}' $LOG_END`
STEAL2=`awk -F " " '{print $9}' $LOG_END`
GUEST2=`awk -F " " '{print $10}' $LOG_END`

SUM1=`expr $USR1 + $NICE1 + $SYS1 + $IDLE1 + $IOWAIT1 + $IRQ1 + $SOFRIRQ1 + $STEAL1 + $GUEST1`

SUM2=`expr $USR2 + $NICE2 + $SYS2 + $IDLE2 + $IOWAIT2 + $IRQ2 + $SOFRIRQ2 + $STEAL2 + $GUEST2`

#USR_PERCENT=`expr \( $USR2 - $USR1 \) \* 100 / \( $SUM2 - $SUM1 \)`
#SYS_PERCENT=`expr \( $SYS2 - $SYS1 \) \* 100 / \( $SUM2 - $SUM1 \)`
#IOWAIT_PERCENT=`expr \( $IOWAIT2 - $IOWAIT1 \) \* 100 / \( $SUM2 - $SUM1 \)`
#IDLE_PERCENT=`expr \( $IDLE2 - $IDLE1 \) \* 100 / \( $SUM2 - $SUM1 \)`

#USR_PERCENT=`expr \( $USR2 - $USR1 \) / 100`
USR_PERCENT=$(echo "scale = 2; ($USR2 - $USR1)" |bc)
SYS_PERCENT=$(echo "scale = 2; ($SYS2 - $SYS1)" |bc)
IOWAIT_PERCENT=$(echo "scale = 2; ($IOWAIT2 - $IOWAIT1)" |bc)
IDLE_PERCENT=$(echo "scale = 2; ($IDLE2 - $IDLE1)" |bc)

# Export value as percentage
echo "$(date --utc +%FT%T.%6N);USR;${USR_PERCENT};SYS;${SYS_PERCENT};IOWAIT;${IOWAIT_PERCENT};IDLE;${IDLE_PERCENT}"
