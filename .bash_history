cd ~
../
cd ../
ls
svn export --force https://internal.cosylab.com/svn/acc/projects/Kyma/NSLS-II-EPU/trunk/apps/trunk/ioc epu1/
ls
cd epu1
ls
cp config_epu1 config
ls
make clean && make
apt-get install epics-iocstats
exit
make clean && make
ls
cd epu1
ls
make clean && make
exit
ls
cd ~
ls
make clean && make
ls
nano config
manage-iocs install epu1
exit
