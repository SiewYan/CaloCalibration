import os
import sys
import subprocess

CWD=os.getcwd()
#SCALEDIR='/home/siew/gm2/df-spectrum/data/scale-opt/campaigne1/opt-range-scale'
SCALEDIR='/home/siew/gm2/df-spectrum/data/scale-opt/campaigne1/opt-range-scale/based-on-region'
DRYRUN=False

def submit_script(jobname_,command_,calo,dryrun=True):
    abspath=os.path.abspath(jobname_)
    exe_script=jobname_

    isRW=True if 'rw_' in command_ else False
    isRW=None if ' -m' in command_ else isRW
    
    # execution script
    with open( exe_script , 'w') as script :
        script.write( '#!/bin/bash\n' )
        script.write( 'source /cvmfs/sft.cern.ch/lcg/views/LCG_102a/x86_64-centos7-gcc11-opt/setup.sh\n' )
        script.write( 'export HOME=%s\n' %(CWD) )
        script.write( 'cd ${HOME}' )
        script.write( '\n' )
        script.write( 'NCALO=$1\n' )
        script.write( 'NXTAL=$2\n' )
        script.write( 'RANGE1=$3 # NOT USED\n' ) # NOT USED
        script.write( 'RANGE2=$4 # NOT USED\n' ) # NOT USED
        script.write( 'SCALE=$5\n' )
        script.write( 'sleep 15\n' )
        script.write( 'echo \"%s\"\n' %(command_) )
        script.write( '%s\n' %(command_) )
        script.close()
    os.system( 'chmod +x %s' %exe_script )

    # submission script
    sub_script = exe_script.replace( '.sh' , '.jdl' )
    with open( sub_script , 'w' ) as script :
        script.write( 'Universe      = vanilla\n' )
        script.write( 'Executable    = %s\n' %(exe_script) )
        script.write( 'Log           = %s\n' %(sub_script.replace('.jdl','_xtal-$(ixtal)-$(Cluster)-$(Process).log')) )
        script.write( 'Output        = %s\n' %(sub_script.replace('.jdl','_xtal-$(ixtal)-$(Cluster)-$(Process).out')) )
        script.write( 'Error         = %s\n' %(sub_script.replace('.jdl','_xtal-$(ixtal)-$(Cluster)-$(Process).err')) )
        script.write( 'request_cpus  = 3\n'  )
        script.write( 'Priority      = 15\n' )
        script.write( 'Rank          = (OpSysName == \"CentOS\")\n')
        script.write( 'Requirements  = (Machine != \"bl-hd-1.phy.sjtulocal\") && (Machine != \"bl-hd-2.phy.sjtulocal\")\n' )
        #script.write( 'queue\n' )
        script.write( 'Arguments     = $(icalo) $(ixtal) $(irange1) $(irange2) $(iscale)\n' )
        
        ### HARDCODED
        if isRW is not None:
            if isRW:
                #script.write( 'queue icalo, ixtal, irange1, irange2, iscale from %s/rw/rw_calo%s_mean_scale.txt\n' %(SCALEDIR,calo) ) #
                script.write( 'queue icalo, ixtal, irange1, irange2, iscale from %s/rw/rw_calo%s_scale.txt\n' %(SCALEDIR,calo) )
            else:
                #script.write( 'queue icalo, ixtal, irange1, irange2, iscale from %s/re/re_calo%s_mean_scale.txt\n' %(SCALEDIR,calo) )
                script.write( 'queue icalo, ixtal, irange1, irange2, iscale from %s/re/re_calo%s_scale.txt\n' %(SCALEDIR,calo) )
        else:
            script.write( 'queue icalo, ixtal, irange1, irange2, iscale from %s/mc/dummy_calo%s.txt\n' %(SCALEDIR,calo) )
            
        #script.write( 'queue ifile from %s\n' %(jobname_) )
        script.close()
        
    if not dryrun: os.system( 'condor_submit %s' %sub_script )
pass
    

def submit(DATALIST_, DATACONDLIST_, ncalo, isMC=False, SUBFOLDER_="/lustre/collider/siew/analysis", OUTROOTFOLDER_="/home/siew/gm2/df-spectrum/data/analysis" ):

    # make outskim folder
    if not os.path.exists(SUBFOLDER_): os.mkdir(SUBFOLDER_)
    if not os.path.exists(OUTROOTFOLDER_): os.mkdir(OUTROOTFOLDER_)
    
    # make some txt
    #f = open( DATALIST_ , "r" )
    #files = [ x for x in f]
    NAME="%s/%s_calo-%s.sh" %( SUBFOLDER_, DATALIST_.split('/')[-1].strip('.txt'), ncalo )

    EXEC="ana"
    COMMAND="%s/bin/%s -d %s -d %s -o %s -c ${NCALO} -x ${NXTAL} -s ${SCALE}" %(
        CWD,
        EXEC,
        DATALIST_,
        DATACONDLIST_,
        OUTROOTFOLDER_)
    if isMC: COMMAND+=" -m"
    print(COMMAND)
    submit_script(NAME,COMMAND,ncalo,DRYRUN)
pass

if __name__ == "__main__":

    if CWD != "/home/siew/gm2/df-spectrum":
        print("run from df-spectrum folder")
        sys.exit(1)

    # "refresh" C++ executable before running
    print('\"refresh\" C++ executable before running')
    try:
        result = subprocess.check_output('make ana', shell=True, text=True)
        print(result)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {e}")
        sys.exit(1)

    # calorimeter
    # inflector area : 23, 24
    # Q1 :  2 , 3 , 4
    # kicker : 5 , 6 , 7
    # Q2 : 8 , 9 , 10
    # open : 1, 11 , 12 , 17 , 18
    # t1 : 13
    # Q3 : 14 , 15 , 16
    # t2 : 19
    # Q4 : 20 , 21 , 22
    # collimater : 9 , 11 , 17 , 21 , 23

    #theCalo=[ 3 , 6 , 9 , 11 , 13 , 15 , 17 , 19 , 21 , 24 ]
    theCalo=[ 1 , 3 , 6 , 13 , 24 ]
    
    # RW
    #for icalo in theCalo:
    #    submit(
    #        "/home/siew/gm2/df-spectrum/data/skim/rw_run4f.txt",
    #        "/home/siew/gm2/df-spectrum/data/fit-result/rw_run4f_cond.txt",icalo)

    # RE
    for icalo in theCalo:
        submit(
            "/home/siew/gm2/df-spectrum/data/skim/re_run4f.txt",
            "/home/siew/gm2/df-spectrum/data/fit-result/re_run4f_cond.txt",icalo)

    # MC
    #for icalo in theCalo:
    #    submit(
    #        "/home/siew/gm2/df-spectrum/data/skim/mc.txt",
    #        "/home/siew/gm2/df-spectrum/data/fit-result/mc_cond.txt",icalo,True)
