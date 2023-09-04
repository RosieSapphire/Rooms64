#!/bin/sh

clear

for i in door door_numplate door_nums locker locker_inside \
	room0 room1 room2 room3 room4 room5
do
	echo $i
	./tools/modelconv/modelconv tools/modelconv/$i.glb
	cp tools/modelconv/$i.mdl filesystem/
done

for i in room0 room1 room2 room3 room4 room5
do
	echo $i
	./tools/roomdatfinder/roomdatfinder ./tools/modelconv/$i.glb
	cp tools/modelconv/$i.dat filesystem/
done

make clean; bear -- make -j8;
