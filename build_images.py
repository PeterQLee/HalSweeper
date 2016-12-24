import os

l=os.listdir('data')

for i in l:
    pre='data/{}'.format(i)
    post=i[:-4]+'o'
    os.system('ld -r -b binary {} -o bin/{}'.format(pre,post))
