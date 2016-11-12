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
import json
from pprint import pprint


data = { 'fav_artists' : ['dsound', 'abba']
         ,'hated_artists' : ['eminem', 'eraserheads', 'garyvalenciano', 'gerihalliwell']
}

def loadJsonFile(json_file):
    data = None
    with open(json_file) as data_file:
        data = json.load(data_file)

    pprint(data)
    return data

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
        return '%s - %s' % (str(self.artist), str(self.title))

    def __repr__(self):
        return '%s - %s' % (str(self.artist), str(self.title))

    def __del__(self):
        pass

def generateKey(artist):
    key = artist.lower()
    key = key.split(' feat')[0]
    key = key.split(' ft.')[0]
    key = ''.join(k for k in key if k.isalnum())

    return key

def isHated(key):
    hatedKeys = data['hated_artists']
    if key in hatedKeys:
        return True
    return False

def getFavoriteArtists(master_list):
    favorite_songs = []
    fav_artists = data['fav_artists']

    for m in master_list:
        if m in fav_artists:
            favorite_songs.append(master_list[m])

    return favorite_songs

def create_serializable(master_list):
    json_serial = {}
    for m in master_list:
        json_serial[m] = []
        mlist = master_list[m]
        for i in mlist:
            json_serial[m].append({'artist':i.artist, 'title':i.title, 'filename':i.filename})

    return json_serial

def readDirectory(directory):
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
            continue


        #print '\n\n --- MP3 INFO --- \n\n'
        #print f.pprint()

        #print 'keys' , f.keys()
        print 'Artist :', f.get('TPE1', None)
        print 'Title  :', f.get('TIT2', None)

        if f == None:
            continue

        artist = str(f.get('TPE1', None))
        title = str(f.get('TIT2', None))

        invalid_artist = ['none', 'unknown']

        if  artist != None and artist != '':
            key = generateKey(artist)

            if key == '':
                continue

            if isHated(key):
                continue

            if key not in invalid_artist:
                if key not in master_list:
                    master_list[key] = []

                master_list[key].append(Song(m, artist,title))

    print 'master_list = %s' % (pformat(master_list))
    master_json = create_serializable(master_list)
    print 'master_json = %s' % (pformat(master_json))
    with open('master_list.json', 'w') as outfile:
        json.dump(master_json, outfile)

    return master_json


def main(argv):
    try:
        master_list = {}
        arg = argv[1]
        if '.json' in arg:
            master_list = loadJsonFile(arg)
        else:
            master_list = None

        if master_list == None:
            

        favorite_songs_by_artists = getFavoriteArtists(master_list)
        print 'favorite_songs_by_artists = %s' % (pformat(favorite_songs_by_artists))

    except Exception, e:
        print 'Exception : ',e






main(sys.argv)
