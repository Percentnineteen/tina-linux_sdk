#! /bin/sh
num=$#
echo "pass $num"
if [ $num -eq 3 ]
then
   COUNT=$3
elif [ $num -eq 4 ]
then
   COUNT=$4
else
   echo "parameter error"
   exit 1
fi
i=0
while [ $i -ne "$COUNT" ]
do
   let i+=1
   echo " ************$i TEST CAMERA************"
   if [ $num -eq 3 ]
   then
      camerapreviewerdemo "$1" "$2" 
   elif [ $num -eq 4 ]
   then
      camerapreviewerdemo "$1" "$2" "$3" 
   else
      echo "parameter error"
      exit 1
   fi

   if [ $? -ne 0 ];
   then
      echo " ************$i TEST ERROR************"
      exit 1
   fi
sleep 1
done
