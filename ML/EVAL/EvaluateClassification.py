import os
import sys
import time

from sklearn import cross_validation, grid_search, \
    metrics, preprocessing, tree
from sklearn.metrics import f1_score, precision_score, recall_score

# python3 EvaluateClassification.py ../../DATA/Datasets/Adult/result.data ../../EXP/ nppvfdt-output.txt


pred=sys.argv[1]
path=sys.argv[2]
res=sys.argv[3]


Y_pred=[]

with open(pred) as f:
    for line in f:
        Y_pred.append(int(line))
Y_test=[]

with open(path+res) as f2:
    for line in f2:
        Y_test.append(int(line))

fmeasure=f1_score(Y_test, Y_pred, average='macro')
precision=precision_score(Y_test, Y_pred, average='macro')
recall=recall_score(Y_test, Y_pred, average='macro')



f = open(path+"Report"+"-"+res, 'w' )

f.write(str(fmeasure))
f.write('\n')
f.write(str(precision))
f.write('\n')
f.write(str(recall))
f.write('\n')
f.close()