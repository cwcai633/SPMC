import json
from collections import defaultdict
import math
import argparse

class MapDealer:
  def __init__(self, user_map, item_map):
    print("Loading...")
    self.user_map = {}
    self.item_map = {}
    for line in open(user_map, 'r'):
      user_entry = line.split()
      user_name = user_entry[0]
      user_id = int(user_entry[1])
      self.user_map[user_id] = user_name
    for line in open(item_map, 'r'):
      item_entry = line.split()
      item_name = item_entry[0]
      item_id = int(item_entry[1])
      self.item_map[item_id] = item_name
    print("Done")

  def translate(self, input_file, output_file):
    ofp = open(output_file, 'w')
    for line in open(input_file, 'r'):
      data = line.split()
      user_id = data[0]
      predicted_item_id = data[1]
      best_friend_id = data[2]
      best_friend_item_id = data[3]
      affinity = data[4]
      test_item_id = data[5]
      ofp.write(self.user_map[int(user_id)])
      ofp.write(', ')
      ofp.write(self.item_map[int(predicted_item_id)])
      ofp.write(', ')
      ofp.write(self.user_map[int(best_friend_id)])
      ofp.write(', ')
      ofp.write(self.item_map[int(best_friend_item_id)])
      ofp.write(', ')
      ofp.write(affinity)
      ofp.write(', ')
      ofp.write(self.item_map[int(test_item_id)])
      ofp.write('\n')

def main(filename):
  md = MapDealer('user_map_30.out', 'item_map_30.out')
  md.translate(filename, filename + '.out')
  

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Translate the visualization file')
  parser.add_argument('filename', type=str)
  args = parser.parse_args()
  main(args.filename)
