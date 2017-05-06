import re
import matplotlib.pyplot as plt
import argparse
import numpy as np
import matplotlib.pylab as pylab
params = {'legend.fontsize': 'x-large',
          'figure.figsize': (16, 11),
         'axes.labelsize': 'x-large',
         #'axes.titlesize':'large',
         'xtick.labelsize':'x-large',
         'ytick.labelsize':'x-large'}
pylab.rcParams.update(params)

def extract(filename):
  aucs = []
  its = []
  pattern = re.compile(r', Test AUC = \d+\.\d*,')
  it = 5
  with open(filename, 'r') as f:
    for line in f:
      match = pattern.findall(line)
      if match:
        auc = re.findall(r'\d+\.\d*', match[0])[0]
        aucs.append(auc)
        its.append(it)
        it += 5
  return its, aucs

def main():
  #parser = argparse.ArgumentParser(description='Plot the learning curve')
  #parser.add_argument('filenames', type=str, nargs='+')
  ##parser.add_argument('name', type=str)
  #args = parser.parse_args()
  #filenames = args.filenames
  #name = args.name
  filenamess =[['bpr_ciao_1_0.01_5.log', 'fpmc_ciao_1_0.01_5.log', 'sbpr_ciao_1_0.01_5.log', 'gbpr_ciao_1_0.01_5.log', 'stbpr_ciao_1_0.1_5.log'], 
               ['bprmf_fs_1_0.01_5_ss.log', 'fpmc_fs_1_0.01_5.log', 'sbpr_fs_1_0.1_5.log', 'gbpr_fs_1_0.01_5_ss.log', 'stbpr_fs_1_0.01_5.log'],
               ['bpr_ep_1_0.01_5.log', 'fpmc_ep_1_0.01_5.log', 'sbpr_ep_1_1_5_ss.log', 'gbpr_ep_1_0.01_5.log', 'stbpr_ep_1_1_5_ss.log'],
               ['bpr_fl_1_0.01_5_ss.log', 'fpmc_fl_1_0.1_5.log', 'sbpr_fl_1_0.1_5.log', 'gbpr_fl_1_0.01_5_ss.log', 'stbpr_fl_1_0.1_5.log']]
  fig = plt.figure()
  i = 1
  names = ['Ciao', 'Foursquare', 'Epinions', 'Flixster']
  for filenames in filenamess:
    p1 = []
    lbl = iter(['BPRMF', 'FPMC', 'SBPR', 'GBPR', 'SPMC'])
    ax = plt.subplot(2, 2, i)
    for filename in filenames:
      its, aucs = extract(filename)
      p1t, = ax.plot(its, aucs, linewidth=2.0, label=lbl.next())
      p1.append(p1t)
    ax.set_xlabel('Iteration') 
    ax.set_ylabel('AUC')
    ax.set_xlim([5, 100])
    plt.title(names[i-1], fontsize=25)
    i += 1
  #plt.show()
  #plt.title(name, fontsize=20)
  plt.subplots_adjust(left=0.07, right=0.95, top=0.95, wspace=0.5, hspace=0.25)
  fig.legend(p1, ['BPRMF', 'FPMC', 'SBPR', 'GBPR', 'SPMC'], loc=8, ncol=5, fontsize='x-large')
  plt.savefig('learning_curve.pdf')

if __name__ == '__main__':
  main()
