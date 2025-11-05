import os

#num_td = [[5, 10], [5, 25], [5, 40],
#          [10, 10], [10, 25], [10, 40], 
#          [15, 10], [15, 25], [15, 40], 
#          [20, 10], [20, 25], 
#          [25, 10], [25, 25]]
num_td = [[5, 5]]
c = 2.5
d = 0.4
e = 1.5
f = 5
seeds = [1762875, 1917698, 1110588, 1767920, 1891798]
for seed in seeds:
    for a, b in num_td:
        command = f'echo \"{a}\n{b}\n{c}\n{d}\n{e}\n{f}\n{seed}\" > settings.settings && python3 ./instance-generator.py < settings.settings && rm -f settings.settings'
        os.system(command)
    #for a in range(3, 10):
    #    for b in range(3, 10):
    #        command = f'echo \"{a}\n{b}\n{c}\n{d}\n{e}\n{f}\n{seed}\" > settings.settings && python3 ./instance-generator.py < settings.settings && rm -f settings.settings'
    #        os.system(command)
    #for a in range(15, 50, 5):
    #    for b in range(15, 50, 5):
    #        command = f'echo \"{a}\n{b}\n{c}\n{d}\n{e}\n{f}\n{seed}\" > settings.settings && python3 ./instance-generator.py < settings.settings && rm -f settings.settings'
    #        os.system(command)