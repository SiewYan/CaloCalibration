import sys
import os.path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from collections import OrderedDict
#sns.set_theme()
sns.set_style("whitegrid",{'grid.linestyle':''})

def fix(df_):

    vlow=1.
    count=0
    for i in range(1, 7):
        ilow = i - vlow
        vhigh=1.
        for j in range(1, 7):
            jup = j - vhigh

            exist=((df_['lfrc'] == round(ilow,2)) & (df_['ufrc'] == round(jup,2))).any()
            #print("ilow : ", round(ilow,2), " ; jup : ", round(jup,2)," is in dataframe : ", exist )
            
            if not exist:
                df_ = df_._append(
                    pd.Series(
                        {
                            'lfrc'  : round(ilow,2),
                            'ufrc'  : round(jup,2),
                            'prob' : 0 if ilow < jup else None
                        }
                    ),
                    ignore_index=True
                )

            vhigh+=0.8
            count+=1;
        vlow+=0.8

    return df_

pass

def ignore(df_, val=[0.5,1.2]):

    for ival in val:
        df_ = df_.replace(ival,np.nan) # np.nan

    return df_
pass

def makeplot(df_,name_,scale_):

    data_matrix = df_.pivot(index="ufrc",columns="lfrc",values="prob")

    # draw heatmap
    f, ax = plt.subplots(figsize=(9, 6))

    hm = sns.heatmap(data_matrix, cmap="viridis", annot=True, fmt='.02f', linewidths=.5, annot_kws={"size": 15}, ax=ax)
    hm.invert_yaxis()
    hm.set_xlabel("Lower bound", fontsize = 15)
    hm.set_ylabel("Upper bound", fontsize = 15)
    hm.figure.axes[-1].set_ylabel('Probability [%]', size=15)
    recon="Recon East" if name_.split('/')[-1].split('_')[0] == "re" else "Recon West" 
    plt.title( "(%s) Optimized Range for %s (mean scale %s)" %(recon,name_.split('/')[-1].split('_')[-1], scale_) , fontsize=15)
    #hm.text(0.6, 0.2, "Mean : ", mean )
    #plt.show()
    plt.savefig( '%s.png' %name_ )
    
    plt.close()
pass

def optimizer(df_, ixtal_, name_, recon_, once=True):

    # selecting the region or xtal
    dff = df_[df_['xtal'].isin(ixtal_)] if isinstance(ixtal_, list) else df_.query('xtal == %s' %ixtal_)
    
    # statistics
    mean = round(dff['scale'].mean(), 3)
    std =  round(dff['scale'].std(), 3)
    print(name_," mean : ", mean)
    print(name_," std  : ", std)

    ##
    #print(dff.query('calo == 1 and lfrc == 0.2 and ufrc == 0.8'))
    # total bound in a given crystal/region (depend on how you sample it)
    dff_total = pd.DataFrame(dff.groupby(['lfrc','ufrc']).size().reset_index(name='total') )
    #print(dff_test)
    #exit()
    
    count=0
    print("optimize in ", name_)
    while dff.shape[0] != 1 :
        count+=1
        lowerb = round( mean - 0.5*0.5*std , 3 )
        upperb = round( mean + 0.5*0.5*std , 3 )

        print("lowerb : ", lowerb, " ; upperb : ", upperb)
        print("dff before : ")
        print(dff)

        dff = dff.query('scale > %s and scale < %s' %(lowerb, upperb))
        #dff = dff.loc[dff["scale"] > lowerb]
        #dff = dff.loc[dff["scale"] < upperb]
        print("dff after : ")
        print(dff)
        
        mean = round(dff['scale'].mean(), 3)
        std =  round(dff['scale'].std(), 3)
        #print("count : ", count," ; df shape : ", dff.shape[0]," ; mean : ", mean, " ; std : ", std)
        if once: break
        if dff.shape[0] == 0: break
        if std <= 0.01 : break

    #print(dff)
    mmean = round(dff['scale'].mean(), 2)
    mstd = round(dff['scale'].std(), 2)
    print("df shape : ", dff.shape[0]," ; mean : ", mmean, " ; std : ", mstd)
    dff_ = pd.DataFrame(dff.groupby(['lfrc','ufrc']).size().reset_index(name='count') )

    # compute percentage
    dff_['prob'] = (dff_['count']/dff_total['total']*100)
    
    dff_plot = fix(dff_)
    #exit()
    outpath='./opt-scale-range/opt-baseOn-region' if isinstance(ixtal_, list) else './opt-scale-range/opt-baseOn-xtal'
    os.system("mkdir -p %s" %outpath)
    makeplot(dff_plot,"%s/%s_%s" %(outpath,recon_, name_),mmean)
    ##
    dff_ = dff_[dff_['prob']!=0].dropna()
    chosen_df = dff_.loc[dff_['prob'].values == dff_['prob'].values.max()]
    chosen_list = chosen_df.to_dict('split')['data']
    #print(chosen_list[0])
    return chosen_list[0]
pass

# get per calo per xtal scale from A,B,C region with respective fix range.
def scale_perCaloXtal(thecalo_, ranges_, dfss_, recon_, region_=None):

    for icalo in thecalo_:

        scaleDIR="./opt-scale-range/opt-baseOn-xtal" if region_ is None else "./opt-scale-range/opt-baseOn-region"
        TXTNAME='%s/%s/%s_calo%s_scale.txt' %(scaleDIR,recon_,recon_,icalo)
        os.system("mkdir -p %s" %os.path.dirname(TXTNAME))
        
        with open( TXTNAME , 'w') as f :
            for ixtal in range(0, 54):

                # get range
                lower=0
                upper=0
                if region_ is not None:
                    for ireg, ival in region_.items():
                        if any(iv == ixtal for iv in ival):
                            lower = ranges_[ireg][0]
                            upper = ranges_[ireg][1]
                            break;
                else:
                    lower = ranges_[ixtal][0]
                    upper = ranges_[ixtal][1]
                    

                # get scale
                print('calo == %s and xtal == %s and lfrc == %s and ufrc == %s' %(icalo, ixtal, lower, upper))
                found_df = dfss_.query('calo == %s and xtal == %s and lfrc == %s and ufrc == %s' %(icalo, ixtal, lower, upper))
                print(found_df)
                #scale  = -999 if found_df.empty else found_df['scale'].item()
                scale  = found_df['scale'].item()
                #scale  = found_df['scale'].item()
                #f.write('%s %s %s %s %s' %(icalo,ixtal,lower*rangev,upper*rangev,round(scale,2)))
                f.write('%s %s %s %s %s' %(icalo,ixtal,lower,upper,round(scale,2)))
                f.write('\n')

pass

# get global scale (mean) from A,B,C region with respective fix range
def scale_global(region_, ranges_, dfss_, recon_):

    #get respective scale , in probably range region ,and crystal region
    meanInReg = {}
    for ireg, ival in region_.items():
        # get probable range in this region
        lower = ranges_[ireg][0]
        upper = ranges_[ireg][1]

        dfsss = dfss[dfss['xtal'].isin(ival)].query('lfrc == %s & ufrc == %s' %(lower,upper))
        #print(dfsss)
        mean = round(dfsss['scale'].mean(), 3)
        std  = round(dfsss['scale'].std(), 3)
        #print('mean : ', mean)
        #print('std  : ', std)
        meanInReg[ireg] = [ mean , std ]

    #print(meanInReg)
    TXTNAME='./opt-scale-range/opt-baseOn-region/%s/%s_global_scale.txt' %(recon_,recon_)
    os.system('mkdir -p %s' %os.path.dirname(TXTNAME))
    with open( TXTNAME , 'w') as f :
        for ixtal in range(0, 54):
            Reg=""
            for ireg, ival in region_.items():
                if any(iv == ixtal for iv in ival):
                    Reg = ireg
                    break; 
                    
            f.write('%s %s %s %s' %(ixtal,ranges_[Reg][0],ranges_[Reg][1],round(meanInReg[Reg][0],2)))
            f.write('\n')
pass

# get global scale (mean) from A,B,C region with respective fix range per calo
def scale_perCalo(thecalo_, ranges_, dfin_, recon_, region_):

    # get respective mean scale, in probable range region and crystal region.
    meanInRegperCalo={}
    for icalo in thecalo_:
        subdict = {}
        for ireg, ival in region_.items():
            lower = ranges_[ireg][0]
            upper = ranges_[ireg][1]

            dfin1 = dfin_[dfin_['xtal'].isin(ival)].query('calo == %s & lfrc == %s & ufrc == %s' %(icalo,lower,upper))
            mean = round(dfin1['scale'].mean(), 3)
            std  = round(dfin1['scale'].std(), 3)
            subdict[ireg] = [ mean , std ]
        meanInRegperCalo[icalo] = subdict
        
    for icalo in thecalo_:
        TXTNAME='./opt-scale-range/opt-baseOn-region/%s/%s_calo%s_mean_scale.txt' %(recon_,recon_,icalo)
        os.system('mkdir -p %s' %os.path.dirname(TXTNAME))
        with open( TXTNAME , 'w') as f :
            for ixtal in range(0, 54):

                # get range
                lower=0
                upper=0
                reg=""
                for ireg, ival in region_.items():
                    if any(iv == ixtal for iv in ival):
                       lower = ranges_[ireg][0]
                       upper = ranges_[ireg][1]
                       reg=ireg
                       break;

                # get scale
                print('calo == %s and xtal == %s and lfrc == %s and ufrc == %s' %(icalo, ixtal, lower, upper))
                found_df = dfin_.query('calo == %s and xtal == %s and lfrc == %s and ufrc == %s' %(icalo, ixtal, lower, upper))
                print(found_df)
                rangev = -999 if found_df.empty else found_df['range'].item()
                #f.write('%s %s %s %s %s' %(icalo,ixtal,lower*rangev,upper*rangev,round(meanInRegperCalo[icalo][reg][0],2)))
                f.write('%s %s %s %s %s' %(icalo,ixtal,lower,upper,round(meanInRegperCalo[icalo][reg][0],2)))
                f.write('\n')

pass

if __name__=="__main__":
    
    # recon
    DIR="/lustre/collider/junkai.ng/CaloCalibration_output/03_scale_scan"
    recon="run"
    RUN="spacer"
    
    # concatenate all the dataframe
    # using all calo ; 
    dfs = []
    #thecalo = [ 1 , 3 , 6 , 9 , 11 , 13 , 15 , 17 , 19 , 21 , 24 ]
    #thecalo = [ 1 , 3 , 6 , 13 , 24 ]
    #thecalo = [ 1 , 3 , 6 , 9 , 13 , 16 , 19 , 22 , 24 ]
    #thecalo = [ 1 ]
    thecalo = [ icalo for icalo in range(1,24) ]
    for icalo in thecalo:
    #for icalo in [ 3 ]:
        for ixtal in range(0,54):
            
            filename = "%s/scale_calo%02d-xtal%02d_%s_%s.txt" %(DIR,icalo,ixtal,RUN,recon)
            print('filename : ', filename)
            
            cols=['calo','xtal','lfrc','ufrc','range','scale','dist']
            df = pd.read_csv(filename, sep=" ", header=None, names=cols)
            #df = ignore(df)
            dfs.append(df)

    dfss = pd.concat(dfs)

    # selection
    #dfss = dfss.drop(dfss[(dfss.lfrc ==0) & (dfss.ufrc == 0.2)].index)
    #dfss = dfss.drop(dfss[(dfss.lfrc ==0) & (dfss.ufrc == 0.4)].index)
    #dfss = dfss.drop(dfss[(dfss.lfrc ==0) & (dfss.ufrc == 0.6)].index)
    #dfss = dfss.drop(dfss[(dfss.lfrc ==0) & (dfss.ufrc == 0.8)].index)
    #dfss = dfss.drop(dfss[(dfss.lfrc ==0) & (dfss.ufrc == 1.0)].index)
    #dfss = dfss.drop(dfss[(dfss.lfrc ==0.6) & (dfss.ufrc == 0.8)].index)
    #dfss = dfss.drop(dfss[(dfss.lfrc ==0.6) & (dfss.ufrc == 1.0)].index)
    #dfss = dfss.drop(dfss[(dfss.lfrc ==0.8) & (dfss.ufrc == 1.0)].index)
   
    dfss_4range = ignore(dfss)

    # selection
    region= {
        "RegionA"  : [ 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 19 , 20 , 21 , 22 , 23 , 24 , 25 , 26 , 28 , 29 , 30 , 31 , 32 , 33 , 34 , 35 , 37 , 38 , 39 , 40 , 41 , 42 , 43 , 44 ],
        "RegionB"  : [ 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 46 , 47 , 48 , 49 , 50 , 51 , 52 , 53 ],
        "RegionC"  : [ 0 , 9 , 18 , 27 , 36 , 45 ]
    }

    #region= {
    #    "RegionA"  : [ 11 , 12 , 13 , 14 , 15 , 16 , 17 , 20 , 21 , 22 , 23 , 24 , 25 , 26 , 29 , 30 , 31 , 32 , 33 , 34 , 35 , 38 , 39 , 40 , 41 , 42 , 43 , 44 ],
    #    "RegionB"  : [ 2 , 3 , 4 , 5 , 6 , 7 , 8 , 47 , 48 , 49 , 50 , 51 , 52 , 53 ],
    #    "RegionC"  : [ 0 , 1 , 9 , 10 , 18 , 19 , 27 , 28 , 36 , 37 , 45 , 46 ]
    #}

    # Get the optimized range (most frequently counted combination of range)
    # base on the region
    ranges_opt = {}
    for ireg in [ "RegionA" , "RegionB" , "RegionC" ]:
    #for ireg in [ "RegionA" ]:
        Reg = region[ireg]
        ranges_opt[ireg] = optimizer( dfss_4range , Reg , ireg , recon )
        #exit()
    print(ranges_opt)
    #exit()
    ##############################################################################
    # produce range and scale list
    # per calo per xtal
    scale_perCaloXtal(thecalo, ranges_opt, dfss, recon, region_=region)
    # per calo mean xtal
    scale_perCalo(thecalo, ranges_opt, dfss, recon, region)
    # mean calo global
    scale_global(region, ranges_opt, dfss, recon )

    ##############################################################################
    # base on xtal global
    xtals_opt= {}
    for ixtal in range(0, 54):
        Xtal_name='Xtal%02d' %ixtal
        xtals_opt[ixtal] = optimizer( dfss_4range , ixtal , Xtal_name, recon )
    print(xtals_opt)

    # individual xtal scale (global)
    scale_perCaloXtal(thecalo, xtals_opt, dfss, recon, region_=None)
