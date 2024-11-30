import os
import sys
import subprocess
from spacer import spacers

CWD=os.getcwd()
DRYRUN=False

def submit_script(jobname_,command_,calo,dryrun=True):
    abspath=os.path.abspath(jobname_)
    exe_script=jobname_
    
    # execution script
    with open( exe_script , 'w') as script :
        script.write( '#!/bin/bash\n' )
        script.write( 'source /cvmfs/sft.cern.ch/lcg/views/LCG_102a/x86_64-centos7-gcc11-opt/setup.sh\n' )
        script.write( 'export HOME=%s\n' %(CWD) )
        script.write( 'cd ${HOME}' )
        script.write( '\n' )
        script.write( 'NCALO=$1\n' )
        script.write( 'NXTAL=$2\n' )
        #script.write( 'make skim\n' )
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
        script.write( 'request_cpus  = 1\n'  )
        script.write( 'Priority      = 15\n' )
        script.write( 'Rank          = (OpSysName == \"CentOS\")\n')
        script.write( 'Requirements  = (Machine != \"bl-hd-1.phy.sjtulocal\") && (Machine != \"bl-hd-2.phy.sjtulocal\")\n' )
        #script.write( 'queue\n' )
        script.write( 'Arguments     = $(icalo) $(ixtal)\n' )
        script.write( 'queue icalo, ixtal from %s/submission/xtal-scheme/calo%s.txt\n' %(CWD,calo) )
        #script.write( 'queue ifile from %s\n' %(jobname_) )
        script.close()
        
    if not dryrun: os.system( 'condor_submit %s' %sub_script )
pass
    

def submit(
        DATALIST_ ,
        DATACONDLIST_ ,
        MCLIST_,
        MCCONDLIST_,
        ncalo ,
        OUTFOLDER_="/lustre/collider/siew/energy-calibration/scale-opt/re"
):

    # make outskim folder
    if not os.path.exists(OUTFOLDER_): os.mkdir(OUTFOLDER_)
    
    # make some txt
    #f = open( DATALIST_ , "r" )
    #files = [ x for x in f]
    NAME="%s/%s_calo-%s.sh" %( OUTFOLDER_, DATALIST_.split('/')[-1].strip('.txt'), ncalo )
    
    #MCLIST_="/home/siew/gm2/df-spectrum/data/skim/mc.txt"
    #MCLIST_="/home/siew/gm2/df-spectrum/data/materials/skim/gasgun.txt"
    #MCCONDLIST_="/home/siew/gm2/df-spectrum/data/materials/fit-result/fit_gasgun_cond.txt"

    #EXEC="scale-opt-re" if re else "scale-opt"
    EXEC="scale-opt"
    COMMAND="%s/bin/%s -d %s -d %s -m %s -m %s -c ${NCALO} -x ${NXTAL}" %(
        CWD,
        EXEC,
        DATALIST_,
        DATACONDLIST_,
        MCLIST_,
        MCCONDLIST_)
    print(COMMAND)
    submit_script(NAME,COMMAND,ncalo,DRYRUN)
pass

if __name__ == "__main__":

    if CWD != "/home/%s/CaloCalibration" %USER :
        print("please run from /home/$s/CaloCalibration" %USER)
        sys.exit(1)

    # "refresh" C++ executable before running
    print('\"refresh\" C++ executable before running')
    try:
        result = subprocess.check_output('make scale-opt', shell=True, text=True)
        result1 = subprocess.check_output('make scale-opt-re', shell=True, text=True)
        print(result)
        print(result1)
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

    theCalo=[ icalo for icalo in range(1,24) ]


    #offset-0p00mm.txt  offset-12p5mm.txt  offset-25p0mm.txt  offset-37p5mm.txt  offset-50p0mm.txt  offset-62p5mm.txt  offset-75p0mm.txt  reference_run.txt  spacer_run.txt
    for icalo in theCalo:

        offset= spacers[icalo].replace( "." , "p" )

        print('calo : ', icalo , " ; offset : ", offset, "mm" )
        
        submit(
            '/home/siew/CaloCalibration/data/skim-files/spacer_run.txt',
            '/home/siew/CaloCalibration/data/fit-results/ ... ',
            '/home/siew/CaloCalibration/data/skim-files/offset-%smm.txt' %offset,
            '/home/siew/CaloCalibration/data/fit-results/offset-%smm_cond.txt' %offset
            icalo
            )
