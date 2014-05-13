alias makeall='make -j4; make -j4 install'
alias smount='mount /dev/sdb1 /mnt/mysdb/; echo noop > /sys/block/sdb/queue/scheduler'
alias sumount='umount /mnt/mysdb/'
alias clcache='echo 3 > /proc/sys/vm/drop_caches'
