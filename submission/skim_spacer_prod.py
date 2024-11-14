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

    # mc
    #for imc in [ 'offset-0p00mm.txt' , 'offset-12p5mm.txt' , 'offset-25p0mm.txt' , 'offset-37p5mm.txt' , 'offset-50p0mm.txt' , 'offset-62p5mm.txt' , 'offset-75p0mm.txt' ]:
    #    skim( "/home/siew/gm2/df-spectrum/data/spacer/art-skim/"+imc , "/lustre/collider/siew/study_v2/spacer/pruned/"+imc.strip('.txt') , "GGAnalyzer/g2phase" , 2 )

    # data
    for	idata in [ 'data-reference.txt' , 'data-spacer.txt' ]:
        skim( "/home/siew/gm2/df-spectrum/data/spacer/art-skim/"+idata , "/lustre/collider/siew/study_v2/spacer/pruned/"+idata.strip('.txt') , "xtalTree/xtal_info" , 3 )
