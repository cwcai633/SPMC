import matplotlib.pyplot as plt
import numpy as np
import csv
import argparse
import mpl_toolkits.axisartist as AA
from mpl_toolkits.axes_grid1 import host_subplot
import matplotlib.pylab as pylab
params = {'legend.fontsize': 'x-large',
          #'figure.figsize': (16, 11),
          'figure.figsize': (32, 7),
         'axes.labelsize': 'x-large',
         #'axes.titlesize':'large',
         'xtick.labelsize':'x-large',
         'ytick.labelsize':'x-large'}
pylab.rcParams.update(params)

def read_csv(filename):
  x = []
  t = []
  n = []
  data = []
  with open(filename,'rb') as csvfile:
    filereader = csv.reader(csvfile)
    for row in filereader:
      n.append(row[0])
      x.append(row[1])
      t.append(row[2])
      data.append(row[3:])
  x = np.array(x)
  t = np.array(t)
  n = np.array(n)
  data = np.array(data)
  return x, t, n, data

def plot(x, t, n, data, filename, ax1):
  lbl = iter(['BPRMF', 'FPMC', 'SBPR', 'GBPR', 'SPMC'])
  #fig, ax1 = plt.subplots()
  #ax1 = host_subplot(111, axes_class=AA.Axes)
  #plt.subplots_adjust(left=0.07, right=0.89, top=0.95, wspace=0.5, hspace=0.25)
  plt.subplots_adjust(left=0.03, right=0.94, top=0.95, bottom=0.16, wspace=0.5, hspace=0.25)
  p1 = []
  for i in range(0, 5):
    p1t, = ax1.plot(n, data[:, i], label=lbl.next(), linewidth=2)
    p1.append(p1t)
    #ax1.plot(x[:-1], data[:, i][:-1], label=lbl.next())
    #ax1.plot(x, data[:, i], label=lbl.next())
  ax2 = ax1.twinx()
  ax3 = ax1.twinx()
  new_fixed_axis = ax3.get_grid_helper().new_fixed_axis
  ax3.axis["right"] = new_fixed_axis(loc="right",
                                      axes=ax3,
                                      offset=(65, 0))
  ax2.axis["right"].toggle(all=True)
  ax1.set_xlabel('Threshold')
  ax1.set_ylabel('AUC')
  ax2.set_ylabel('Avg. Trusts')
  ax3.set_ylabel('Avg. Items')
  p2, = ax2.plot(n, t, 'g--', label='Avg. Trusts')
  p3, = ax3.plot(n, x, 'r--', label='Avg. Items') 
  p1.append(p2)
  p1.append(p3)
  #ax2.plot(x[:-1], t[:-1], 'k--', label='Avg. Trusts')
  #ax2.plot(x, t, 'k--', label='Avg. Trusts')
  ax2.axis["right"].label.set_color(p2.get_color())
  ax3.axis["right"].label.set_color(p3.get_color())
  #ax1.legend(loc=4, fontsize=14)
  #ax2.legend(loc=8)
  plt.title(filename.split('.')[0], fontsize=25)
  return p1, p2, p3
   
def main():
  #parser = argparse.ArgumentParser(description='Plot from CSV files')
  #parser.add_argument('filename', type=str)
  #args = parser.parse_args()
  #filename = args.filename
  filenames = ['Ciao.csv', 'Foursquare.csv', 'Epinions.csv', 'Flixster.csv']
  i = 1
  #fig = plt.figure(figsize=(14, 10))
  fig = plt.figure()
  for filename in filenames:
    x, t, n, data = read_csv(filename)
    ax = host_subplot(1, 4, i, axes_class=AA.Axes)
    p1, p2, p3 = plot(x, t, n, data, filename, ax) 
    i += 1
  fig.legend(p1, ['BPRMF', 'FPMC', 'SBPR', 'GBPR', 'SPMC', 'Avg. Trusts', 'Avg. Items'], loc=8, ncol=7, fontsize='x-large')
  plt.savefig('auc.pdf')

if __name__ == '__main__':
  main()
