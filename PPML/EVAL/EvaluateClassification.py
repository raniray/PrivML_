import warnings
warnings.filterwarnings("ignore")
import sys
from sklearn.metrics import f1_score, precision_score, recall_score

# python3 EvaluateClassification.py ../../DATA/Datasets/Adult/result.data ../../EXP/ nppvfdt-output.txt ../../EXP/classificationReport


pred=sys.argv[1]
path=sys.argv[2]
res=sys.argv[3]
test_size=int(sys.argv[4].strip())



Y_pred=[]
cpt=0;
with open(pred) as f:
    for line in f:
        if cpt<test_size:
            Y_pred.append(int(line))
            cpt=cpt+1
        else:
            break
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