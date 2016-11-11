#!/usr/bin/env python

#from mutagen._senf import print_, argv
#from mutagen._compat import text_type

#from ._util import SignalHandler, OptionParser

import sys
import glob
from mutagen import File
import os
import fnmatch
import os
from pprint import *



def getMp3Files (directory):
    matches = []
    for root, dirnames, filenames in os.walk(directory):
        for filename in fnmatch.filter(filenames, '*.mp3'):
            matches.append(os.path.join(root, filename))

    return matches

class Song():
    def __init__(self, filename, artist, title):
        self.filename = filename
        self.artist = artist
        self.title = title

    def __str__(self):
        return '%s - %s' % (self.artist, self.title)

    def __repr__(self):
        return '%s - %s' % (self.artist, self.title)
        
    def __del__(self):
        pass

def main(argv):
    try:
        master_list = {}
        directory = argv[1]
        mp3s = getMp3Files(directory)
        for m in mp3s:
            f = None
            try:
                f = File(m)
            except:
                print 'Error occurred on %s' % m
                if f != None:
                    print f.pprint()


            #print '\n\n --- MP3 INFO --- \n\n'
            #print f.pprint()

            #print 'keys' , f.keys()
            #print 'Artist :', f.get('TPE1', None)
            #print 'Title  :', f.get('TIT2', None)

            if f == None:
                continue

            artist = str(f.get('TPE1', None))
            title = str(f.get('TIT2', None))

            invalid_artist = ['NONE', 'UNKNOWN']

            if  artist != None and artist.upper() not in invalid_artist:

                #print '%s - %s' % (artist, title)
                if artist not in master_list:
                    master_list[artist] = []

                master_list[artist].append(Song(m, artist,title))

        print 'master_list = %s' % (pformat(master_list))

    except Exception, e:
        print 'Exception : ',e






main(sys.argv)
