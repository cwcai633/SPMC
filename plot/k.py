import matplotlib.pyplot as plt
import numpy as np
import matplotlib.pylab as pylab
params = {'legend.fontsize': 'x-large',
          #'figure.figsize': (16, 11),
          'figure.figsize': (16, 11),
         'axes.labelsize': 'x-large',
         #'axes.titlesize':'large',
         'xtick.labelsize':'x-large',
         'ytick.labelsize':'x-large'}
pylab.rcParams.update(params)

x = [5, 10, 20, 40]
data = [[0.490639, 0.494161, 0.593383, 0.607555],
        [0.889691, 0.894108, 0.887596, 0.871286],
        [0.543112, 0.575129, 0.595317, 0.602357],
        [0.900143, 0.90043, 0.900146, 0.901381]]
lbl = iter(['Ciao', 'Foursquare', 'Epinions', 'Flixster'])

fig = plt.subplots(2, 2)
i = 1
for d in data:
  l = lbl.next()
  ax = plt.subplot(2, 2, i)
  ax.plot(x, d, marker='o', linewidth=2, label=l)
  ax.set_xlabel('Dimension')
  ax.set_ylabel('AUC')
  plt.title(l, fontsize=25)
  i += 1
plt.subplots_adjust(left=0.07, right=0.95, top=0.9, wspace=0.5, hspace=0.25)
#plt.suptitle(r'The impact of different Dimension K', fontsize='xx-large')
#ax.set_xlim([0.5, 1.5])
#plt.show()
plt.savefig('k.pdf')
