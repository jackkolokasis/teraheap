#!/usr/bin/env bash                                                                     

before=()                                                                       
after=()                                                                        
dimms=()                                                                        
while IFS== read -r line1 line2                                                 
do                                                                              
	if [[ "${line1}" == -* ]]                                                   
	then                                                                        
		dimms+=( ${line1}+${line2} )                                            
	else                                                                        
		before+=( ${line2} )                                                    
	fi                                                                          
done <pmem_before.txt                                                           

while IFS== read -r line1 line2                                                 
do                                                                              
    if [[ "${line1}" != -* ]]                                                   
    then                                                                        
        after+=( ${line2} )                                                     
    fi                                                                          
done <pmem_after.txt                                                            

mr=0                                                                            
mw=0                                                                            
rr=0                                                                            
wr=0                                                                            
tmr=0                                                                           
tmw=0                                                                           
trr=0                                                                           
twr=0                                                                           

for i in "${!before[@]}"; do                                                    
    if [ $(( $i % 8 )) -eq 0 ]; then                                            
        echo "${dimms[$i / 8]}"                                                 
        echo ""                                                                 
    fi                                                                          
    b=${before[$i]}                                                             
    a=${after[$i]}                                                              
    res=$(( $a - $b ))                                                          
    if [ $(( $i % 8 )) -eq 0 ]; then                                            
        echo "MediaReads=" "${res}"                                             
        echo ""                                                                 
        (( mr += res ))                                                         
    fi                                                                          
    if [ $(( $i % 8 )) -eq 1 ]; then                                            
        echo "MediaWrites=" "${res}"                                            
        echo ""                                                                 
        (( mw += res ))                                                         
    fi                                                                          
    if [ $(( $i % 8 )) -eq 2 ]; then                                            
        echo "ReadRequests=" "${res}"                                           
        echo ""                   
		        (( rr += res ))                                                         
    fi                                                                          
    if [ $(( $i % 8 )) -eq 3 ]; then                                            
        echo "WriteRequests=" "${res}"                                          
        echo ""                                                                 
        (( wr += res ))                                                         
    fi                                                                          
    if [ $(( $i % 8 )) -eq 4 ]; then                                            
        echo "TotalMediaReads=" "${res}"                                        
        echo ""                                                                 
        (( tmr += res ))                                                        
    fi                                                                          
    if [ $(( $i % 8 )) -eq 5 ]; then                                            
        echo "TotalMediaWrites=" "${res}"                                       
        echo ""                                                                 
        (( tmw += res ))                                                        
    fi                                                                          
    if [ $(( $i % 8 )) -eq 6 ]; then                                            
        echo "TotalReadRequests=" "${res}"                                      
        echo ""                                                                 
        (( trr += res ))                                                        
    fi                                                                          
    if [ $(( $i % 8 )) -eq 7 ]; then                                            
        echo "TotalWriteRequests=" "${res}"                                     
        echo ""                                                                 
        (( twr += res ))                                                        
    fi                                                                          
done                                                                            
                                                                                
echo "TOTAL:"                                                                   
echo ""                                                                         
echo "MediaReads=" "${mr}"                                                      
echo ""                                                                         
echo "MediaWrites=" "${mw}"                                                     
echo ""                                                                         
echo "ReadRequests=" "${rr}"                                                    
echo ""                                                                         
echo "WriteRequests=" "${wr}"                                                   
echo ""                                                                         
echo "TotalMediaReads=" "${tmr}"                                                
echo ""                                                                         
echo "TotalMediaWrites=" "${tmw}"                                               
echo ""                                                                         
echo "TotalReadRequests=" "${trr}"                                              
echo ""                                                                         
echo "TotalWriteRequests=" "${twr}"                                             
echo ""
