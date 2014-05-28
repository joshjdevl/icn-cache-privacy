import csv
with open('consumernodeid-0.timestamp-0.stat', 'rb') as f:
    reader = csv.reader(f,delimiter=' ')
    for row in reader:
        print row[0]
