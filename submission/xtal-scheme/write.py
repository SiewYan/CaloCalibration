#!/bin/python

#with open('flatten_calo.txt', 'w') as f :
#    for icalo in range(1,25):
#        for ixtal in range(0, 54):
#            f.write('%s, %s' %(icalo,ixtal))
#            f.write('\n')


for icalo in range(1,25):
    
    with open( 'calo%s.txt' %icalo , 'w') as f :
        for ixtal in range(0, 54):
            f.write('%s, %s' %(icalo,ixtal))
            f.write('\n')
