#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.

from twisted.trial import unittest
import os
import shutil
import copy

import addressbook
from addressbook import AddressBook, Contact, ip_range
from errors import AddressBookError
from utils.i18n import to_utf
import utils.log

ALPHABET = 'abcdefghijklmnopqrstuvwxyz_@ ABCDEFÙ1234\'2345-678"9."0!?=+%$()[]{}#<>€£éèêëà§çπ‡Ò∂ƒﬁ~'
UALPHABET = u'2abcdefghijklmnopqrstuvwxyz_@ ABCDEFÙ1234\'2345-678"9."0!?=+%$()[]{}#<>€£éèêëà§çπ‡Ò∂ƒﬁ~'

del addressbook.log
addressbook.log = utils.log.start('error', 1, 0, 'adb')


            
class Test_1_Ip_range(unittest.TestCase):
    
    def test_ip_range(self):
        addresses = {'123.123.123.123':True,
                     u'123.123.123.123':True,
                     '423.123.123.123':False,
                     '123.123.0.123':True,
                     '123.-1.123.123':False
                     }
    
        for address, result in addresses.items():
            try:
                test_result = ip_range(address)
            except AddressBookError:
                test_result = False
            self.assertEqual(test_result, result,
                             'Problem testing IP range for this address: %s. (%s, %s)' % (address, result, test_result))
        
  
class Test_2_Contact(unittest.TestCase):
    
    def test_1_name(self):
        names = {'':False,
                 'a':True,
                 ' ':True,
                 'Etienne':True,
                 ALPHABET:True,
                 u'Etienne':True,
                 UALPHABET:True
                 }
        
        for name, result in names.items():
            try:
                contact = Contact(name, []) # put a list as address to be sure to validate in set_kind
                if contact:
                    test_result = True
            except AddressBookError:
                test_result = False
            else:
                self.assertEqual(contact.name, name, 'In and out name doesn\'t match: %s -> %s.' % (name, contact.name))
                del contact

            self.assertEqual(test_result, result, 'Problem validating name: %s. (%s, %s)' % (name, test_result, result))
            
    def test_2_kind_address(self):
        addresses = (('', False),
                     ('240.123.123.123', True, 'ip'),
                     ('23.123.45.222', True, 'ip'),
                     ('23.423.45.222', False),
                     ('233.123.45.222', True, 'multicast'),
                     ([], True, 'group'),
                     ('345@23.123.45.222', True, 'sip_ip'),
                     ('gros toto@23.123.45.222', False),
                     ('etienne@23.123.45.222', True, 'sip_ip'),
                     ('George@amuse.toi.com', True, 'sip_name'),
                     ('Éloi@23.123.45.222', False),
                     (u'Étienne@23.123.45.222', False),
                     (u'23.123.45.222', True, 'ip'),
                     (u'Jean@23.123.45.222', True, 'sip_ip')
                     )
        
        for address in addresses:
            try:
                contact = Contact('a', address[0])
                if contact:
                    test_result = True
            except AddressBookError:
                test_result = False
            else:
                self.assertEqual(contact.kind, address[2], 'Bad kind for this address: %s. (%s, %s)' % (address[0], contact.kind, address[2]))
                self.assertEqual(contact.address, address[0], 'In and out not matching for this address: %s -> %s.' % (address[0], contact.kind))
                del contact
                
            self.assertEqual(test_result, address[1], 'Problem validating address: %s. (%s, %s)' % (address[0], test_result, address[1]))
                    
    def test_3_port(self):
        ports = (('', True),
                 (0, True),
                 ('0', True),
                 (u'0', True),
                 ('2000', True),
                 (u'3466', True),
                 (44575, True),
                 ('1000', False),
                 (u'1', False),
                 (- 3, False),
                 ('200000', False),
                 (u'65536', False),
                 (445750, False),
                 ([23532], False),
                 ('2test5', False),
                 (u'34ten5', False)
                 )
        
        for port in ports:
            try:
                contact = Contact('ab', [], port[0]) # put a list as address to be sure to validate in set_kind
                if contact:
                    test_result = True
            except AddressBookError:
                test_result = False
            else:
                if port[0] in ('', 0, '0'):
                    self.assertEqual(contact.port, None, 'contact.port should be None. Get: %s.' % contact.port)
                else:
                    self.assertEqual(contact.port, int(port[0]), 'In and out port doesn\'t match: %s -> %s.' % (port[0], contact.port))
                del contact

            self.assertEqual(test_result, port[1], 'Problem validating port: %s. (%s, %s)' % (port[0], test_result, port[1]))
   
    def test_4_connector(self):
        # with contact.connector set to None
        connectors = (('test', [], 'test'),
                     (None, [], None),
                     (None, '123.123.123.13', 'basic'),
                     (None, 'john@123.123.123.123', 'sip'),
                     (None, 'best@vroumm.org', 'sip'),
                     (None, '230.23.23.23', 'basic')
                     )
        
        for connector in connectors:
            contact = Contact('ab', connector[1], connector=connector[0]) # put a list as address to be sure to validate in set_kind
            self.assertEqual(contact.connector, connector[2], 'In and out connector doesn\'t match: %s should give %s.' % (contact.connector, connector[2]))
            del contact

        # with contact.connector set to 'other'
        connectors = (('test', [], 'test'),
                     (None, [], 'other'),
                     (None, '123.123.123.13', 'other'),
                     (None, 'john@123.123.123.123', 'other'),
                     (None, 'best@vroumm.org', 'other'),
                     (None, '230.23.23.23', 'other')
                     )
        
        for connector in connectors:
            contact = Contact('ab', connector[1], connector='other') # put a list as address to be sure to validate in set_kind
            contact.assign_connector(connector[0])
            self.assertEqual(contact.connector, connector[2], 'In and out connector doesn\'t match: %s should give %s.' % (contact.connector, connector[2]))
            del contact

    def test_5_setting(self):
        settings = {0:True,
                   1:True,
                   122:True,
                   '123.123.0.123':False
                   }
    
        for setting, result in settings.items():
            try:
                contact = Contact('ab', [], setting=setting) # put a list as address to be sure to validate in set_kind
                if contact:
                    test_result = True
            except AddressBookError:
                test_result = False
            else:
                self.assertEqual(contact.setting, setting, 'In and out not matching for this address: %s -> %s.' % (setting, contact.setting))
                del contact
                
            self.assertEqual(test_result, result, 'Problem validating address: %s. (%s, %s)' % (setting, test_result, result))
            

class Test_3_AddressBook(unittest.TestCase):
    
    filenames = {'':False,
                  u'':False,
                  'a':True,
                  u'b':True,
                  ' c':True,
                  u' d':True,
                  ALPHABET:True,
                  UALPHABET:True
                  }
    
    base_file = 'test'

    def setUp(self):
        self.orig_home = os.environ['HOME']
        os.environ['HOME'] = '/var/tmp'

    def tearDown(self):
        for filename in self.filenames:
            strip_filename = filename.lstrip()
            if strip_filename:
                shutil.rmtree(os.environ['HOME'] + '/.' + strip_filename, True)
        shutil.rmtree(os.environ['HOME'] + '/.' + self.base_file, True)
        os.environ['HOME'] = self.orig_home        
        
    def test_1_init(self):
        for filename, result in self.filenames.items():
            strip_filename = filename.lstrip()
            try:
                adb = AddressBook(filename)
                if adb:
                    test_result = True
            except AddressBookError:
                test_result = False
            else:
                adb_filename = to_utf(os.environ['HOME'] + '/.' + strip_filename + '/' + strip_filename + '.adb')
                self.assertEqual(adb.filename, adb_filename, 'In and out filename not matching: %s -> %s' % (adb_filename, adb.filename))
                del adb
            
            self.assertEqual(result, test_result, 'Problem validating AddressBook creation. <%s> %s:%s' % (filename, result, test_result))

    def test_2_read_write_add(self):
        for filename, result in self.filenames.items():
            try:
                adb = AddressBook(filename)
            except AddressBookError:
                pass
            else:
                result = adb.add('Jean', []) # Should add at least one contact to write the file. Very simple contact.
                if result:
                    in_adb = copy.copy(adb.contacts)
                    adb.read()
                    self.assertEqual(in_adb.keys(), adb.contacts.keys(), 'In and out contacts list not matching: %s -> %s' % (in_adb.keys(), adb.contacts.keys()))
                    for contact in adb.contacts.keys():
                        if isinstance(in_adb[contact], Contact):
                            self.assertEqual(in_adb[contact].__dict__, adb.contacts[contact].__dict__, 'In and out contacts list not matching: %s -> %s' % (in_adb, adb.contacts))
                    
                    self.assertRaises(AddressBookError, adb.add, 'Jean', [])
                del adb
                        
    def test_3_select(self):
        try:
            adb = AddressBook(self.base_file)
            adb.add('Jules', [])
        except AddressBookError:
            pass
        else:
            self.assertRaises(AddressBookError, adb.select, 'Marion')
            adb.select('Jules')
            self.assertEqual(adb.contacts['_selected'], 'Jules', 'Selection of contact did not work: selected \'Jules\' got %s' % adb.contacts['_selected'])
            
    def test_4_get_current(self):
        try:
            adb = AddressBook(self.base_file)
            adb.add('Jules', [])
        except AddressBookError:
            pass
        else:
            self.assertEqual(adb.get_current(), None, 'Selected contact should be None, got %s' % adb.get_current())
            adb.select('Jules')
            self.assertEqual(adb.get_current(), adb.contacts['Jules'], 'Selected contact should be \'Jules\', got %s' % adb.get_current())
            
    def test_5_delete(self):
        try:
            adb = AddressBook(self.base_file)
            adb.add('Jules', [])
            adb.select('Jules')
            adb.add('Guenièvre Temps-Dur', [])
        except AddressBookError:
            pass
        else:
            self.assertRaises(AddressBookError, adb.delete, 'Vlimeux')
            adb.delete('Jules')
            self.assertNotIn('Jules', adb.contacts)
            self.assertEqual(adb.contacts['_selected'], None, 'After removing the selected contact, selected should be None, got %s' % adb.contacts['_selected'])
            self.assertRaises(AddressBookError, adb.delete)
            adb.select('Guenièvre Temps-Dur')
            adb.delete()          
            self.assertNotIn('Guenièvre Temps-Dur', adb.contacts)
            self.assertEqual(adb.contacts['_selected'], None, 'After removing the selected contact, selected should be None, got %s' % adb.contacts['_selected'])
            
    def test_6_duplicate(self):
        try:
            adb = AddressBook(self.base_file)
            adb.add('Jules', [])
            adb.add('Guenièvre Temps-Dur', [])
        except AddressBookError:
            pass
        else:
            self.assertRaises(AddressBookError, adb.duplicate)
            adb.select('Jules')
            adb.duplicate()
            self.assertIn('Jules' + adb.dup_suffix, adb.contacts)
            self.assertRaises(AddressBookError, adb.duplicate, 'Vlimeux')
            adb.duplicate('Guenièvre Temps-Dur')
            self.assertIn(to_utf('Guenièvre Temps-Dur') + adb.dup_suffix, adb.contacts)
            
            adb.duplicate(new_name='dup_test')
            self.assertIn('dup_test', adb.contacts)
            self.assertRaises(AddressBookError, adb.duplicate, new_name='Jules')
        
    def test_7_modify(self):
        try:
            adb = AddressBook(self.base_file)
            adb.add('Jules', [])
            adb.add('Guenièvre Temps-Dur', [])
        except AddressBookError:
            pass
        else:
            self.assertRaises(AddressBookError, adb.modify)
            adb.select('Jules')
            orig_contact = copy.copy(adb.get_current())
            adb.modify()
            self.assertEqual(orig_contact.__dict__, adb.get_current().__dict__, 'The contact is not supposed to be modify but it is.')
            adb.modify('Jules')
            self.assertEqual(orig_contact.__dict__, adb.get_current().__dict__, 'The contact is not supposed to be modify but it is.')
            self.assertRaises(AddressBookError, adb.modify, 'Guenièvre Temps-Dur', 'Jules')
            orig_contact = adb.contacts[u'Guenièvre Temps-Dur']
            adb.modify('Guenièvre Temps-Dur', 'Blouarp Mégadef')
            self.assertIdentical(orig_contact, adb.contacts[u'Blouarp Mégadef'], 'The contact object is supposed to be same before and after a modify. It is not.')
            adb.modify('Blouarp Mégadef', address='123.123.123.123', port=1234)
            contact = adb.contacts[u'Blouarp Mégadef']
            self.assertEqual(contact.address, '123.123.123.123', 'Address was not change properly in the contact. %s -> %s' % ('123.123.123.123', contact.address))
            self.assertEqual(contact.port, 1234, 'Port was not change properly in the contact. %s -> %s' % (1234, contact.port))
            adb.modify('Blouarp Mégadef', 'Mulfist', address='simon@atom.zong.org', port='1234')
            contact = adb.contacts['Mulfist']
            self.assertEqual(contact.address, 'simon@atom.zong.org', 'Address was not change properly in the contact. %s -> %s' % ('simon@atom.zong.org', contact.address))
            self.assertEqual(contact.port, 1234, 'Port was not change properly in the contact. %s -> %s' % (1234, contact.port))
       
