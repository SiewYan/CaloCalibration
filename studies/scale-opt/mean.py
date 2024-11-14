import sys

import numpy as np
import pandas as pd

if __name__=="__main__":
    
    filename = sys.argv[1]

    column=['scale']
    df = pd.read_csv(filename, sep=" ", header=None, names=column)
    
    # statistics
    mean = round(df['scale'].mean(), 3)
    std =  round(df['scale'].std(), 3)
    print(mean)
