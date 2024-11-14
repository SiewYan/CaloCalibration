import os
import sys
import subprocess

CWD=os.getcwd()

def submit_script(jobname_,command_,dryrun=True):
    abspath=os.path.abspath(jobname_)
    exe_script=jobname_.replace('.txt','.sh')
    
    # execution script
    with open( exe_script , 'w') as script :
        script.write( '#!/bin/bash\n' )
        script.write( 'source /cvmfs/sft.cern.ch/lcg/views/LCG_102a/x86_64-centos7-gcc11-opt/setup.sh\n' )
        script.write( 'export HOME=%s\n' %(CWD) )
        script.write( 'cd ${HOME}' )
        script.write( '\n' )
        #script.write( 'make skim\n' )
        script.write( 'sleep 15\n' )
        script.write( 'echo \"%s\"\n' %(command_) )
        script.write( '%s\n' %(command_) )
        script.close()
    os.system( 'chmod +x %s' %exe_script )

    # submission script
    sub_script = exe_script.replace( '.sh' , '.jdl' )
    ncore = command_.split('-n ')[-1]
    with open( sub_script , 'w' ) as script :
        script.write( 'Universe      = vanilla\n' )
        script.write( 'Executable    = %s\n' %(exe_script) )
        script.write( 'Log           = %s\n' %(sub_script.replace('.jdl','.log')) )
        script.write( 'Output        = %s\n' %(sub_script.replace('.jdl','.out')) )
        script.write( 'Error         = %s\n' %(sub_script.replace('.jdl','.err')) )
        script.write( 'request_cpus  = %s\n' %(ncore) )
        script.write( 'Priority      = 15\n' )
        script.write( 'Rank          = (OpSysName == \"CentOS\")\n')
        script.write( 'Requirements  = (Machine != \"bl-hd-1.phy.sjtulocal\") && (Machine != \"bl-hd-2.phy.sjtulocal\")\n' )
        script.write( 'queue\n' )
        #script.write( 'Arguments     = $(ifile)\n' )
        #script.write( 'queue ifile from %s\n' %(jobname_) )
        script.close()
        
    if not dryrun: os.system( 'condor_submit %s' %sub_script )
pass
    

def skim(ARTSKIMLIST_, OUTSKIM_, TREENAME_="GGAnalyzer/g2phase", LINELIMIT_=3, NCORE=2):

    # make outskim folder
    if not os.path.exists(OUTSKIM_): os.mkdir(OUTSKIM_)
    
    # make some txt
    f = open( ARTSKIMLIST_ , "r" )
    files = [ x for x in f]
    NAME=ARTSKIMLIST_.split('/')[-1].strip('.txt')

    count=0
    subcount=0
    lines=[]
    for ifile in files:
        count+=1
        lines.append(ifile)
    
        if len(lines) == LINELIMIT_ or count == len(files):
            subcount+=1
            #print( "save subfilelist at %s/%s_%02d.txt" %(OUTSKIM_, NAME, subcount) )
            subfile = open( '%s/%s_%02d.txt' %(OUTSKIM_, NAME, subcount) , 'w' )
            subfile.writelines(lines)
            subfile.close()
            lines.clear()
        
    # submission
    subflist = os.listdir(OUTSKIM_)
    for yfile in sorted(subflist):
        if '.txt' not in yfile: continue
        subfile_in='%s/%s' %(OUTSKIM_,yfile)
        if 'ntuple' in TREENAME_:
            COMMAND="%s/bin/skim -f %s -o %s -t %s -x -n %s" %(CWD,subfile_in,subfile_in.replace('.txt','.root'),TREENAME_,NCORE)
        else:
            COMMAND="%s/bin/skim -f %s -o %s -t %s -n %s" %(CWD,subfile_in,subfile_in.replace('.txt','.root'),TREENAME_,NCORE)
        print(COMMAND)
        submit_script(subfile_in,COMMAND,False)
pass

if __name__ == "__main__":

    if CWD != "/home/siew/gm2/df-spectrum":
        print("run from df-spectrum folder")
        sys.exit(1)

    # "refresh" C++ executable before running
    print('\"refresh\" C++ executable before running')
    try:
        result = subprocess.check_output('make skim', shell=True, text=True)
        print(result)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {e}")
        sys.exit(1)
        
    # MC
    #skim( "/home/siew/gm2/df-spectrum/data/art-skim/gasgun_1b.txt" , "/lustre/collider/siew/pruned_datasets/MC_v2" , "GGAnalyzer/g2phase" , 5 )
    #skim( "/home/siew/gm2/df-spectrum/data/materials/art-skim/gasgun.txt" , "/lustre/collider/siew/energy-calibration/gasgun" , "gAnalyzer/g2phase" , 5 )
    
    # RW
    #skim( "/home/siew/gm2/df-spectrum/data/materials/art-skim/run4F_RW.txt" , "/lustre/collider/siew/energy-calibration/run4F_rw" , "xtalTree/xtal_info" , 1 )
    #skim( "/home/siew/gm2/df-spectrum/data/materials/art-skim/run4O_RW.txt" , "/lustre/collider/siew/energy-calibration/run4O_rw" , "xtalTree/xtal_info" , 1 )
    #skim( "/home/siew/gm2/df-spectrum/data/materials/art-skim/run5A_RW.txt" , "/lustre/collider/siew/energy-calibration/run5A_rw" , "xtalTree/xtal_info" , 1 )
    #skim( "/home/siew/gm2/df-spectrum/data/materials/art-skim/run5BC_RW.txt" , "/lustre/collider/siew/energy-calibration/run5BC_rw" , "xtalTree/xtal_info" , 1 )
    #skim( "/home/siew/gm2/df-spectrum/data/materials/art-skim/run5P_RW.txt" , "/lustre/collider/siew/energy-calibration/run5P_rw" , "xtalTree/xtal_info" , 1 )
    
    # RE
    #skim( "/home/siew/gm2/df-spectrum/data/art-skim/reconEast_Run4F.txt" , "/lustre/collider/siew/pruned_datasets/RE_Run4F" , "crystalTreeMaker1EP/ntuple" )
    skim( "/home/siew/gm2/df-spectrum/data/materials/art-skim/run4F_RE.txt" , "/lustre/collider/siew/energy-calibration/run4F_re" , "crystalTreeMaker1EP/ntuple" )
    skim( "/home/siew/gm2/df-spectrum/data/materials/art-skim/run5A_RE.txt" , "/lustre/collider/siew/energy-calibration/run5A_re" , "crystalTreeMaker1EP/ntuple" )
