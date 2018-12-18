#!/bin/bash
set -x
DISK=/dev/sdc
SCSI=`ls /sys/block/${DISK:5:3}/device/scsi_device/`

mkdir -p /mnt/xfs

while true; do

        echo "mkfs on $DISK"
        mkfs.xfs -f $DISK
        sleep 1;
        echo "mount and IO on $DISK"
        mount $DISK /mnt/xfs
        dd if=/dev/zero of=/mnt/xfs/file1 bs=1M count=500
        sync
        sleep 1
        echo "offline $DISK"
        echo 1 > /sys/block/${DISK:5:3}/device/delete
        xfs_io -x -c shutdown /mnt/xfs
        echo "umount..."
        while ( ! umount /mnt/xfs ) && [ -n "`cat /proc/mounts | egrep -e \"/mnt/xfs\"`" ] ; do echo "retrying in 5 sec."; sleep 5; done
        echo "done"
        sleep 3;
        echo "online $SCSI"
        echo "- - -" > /sys/class/scsi_host/host${SCSI:0:1}/scan
        sleep 5
        DISK=/dev/`ls /sys/class/scsi_device/$SCSI/device/block`
        echo "new disk name: $DISK"
done
set +x

#trace-cmd start -e "xfs:*"
#cat /sys/kernel/debug/tracing/trace_pipe
