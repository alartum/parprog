import sys

data = []
with open(sys.argv[1], "r") as f:
    for line in f:
        fields = line.split()
        rowdata = map(float, fields)
        data.extend(rowdata)

print('{}'.format(sum(data)/len(data)))
