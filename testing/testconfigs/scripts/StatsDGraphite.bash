###############################################################################  
#University of Illinois/NCSA Open Source License
#Copyright (c) 2015 Information Trust Institute
#All rights reserved.
#
#Developed by: 		
#
#Information Trust Institute
#University of Illinois
#http://www.iti.illinois.edu
#
#Permission is hereby granted, free of charge, to any person obtaining a copy of
#this software and associated documentation files (the "Software"), to deal with
#the Software without restriction, including without limitation the rights to 
#use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
#of the Software, and to permit persons to whom the Software is furnished to do 
#so, subject to the following conditions:
#
#Redistributions of source code must retain the above copyright notice, this list
#of conditions and the following disclaimers. Redistributions in binary form must 
#reproduce the above copyright notice, this list of conditions and the following
#disclaimers in the documentation and/or other materials provided with the 
#distribution.
#
#Neither the names of Information Trust Institute, University of Illinois, nor
#the names of its contributors may be used to endorse or promote products derived 
#from this Software without specific prior written permission.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
#FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS 
#OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
#WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
#CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
###############################################################################  
sudo apt-get install graphite-web graphite-carbon sqlite3 npm nodejs apache2 libapache2-mod-wsgi
sudo graphite-manage syncdb --settings=graphite.settings

sudo chown _graphite /var/lib/graphite/graphite.db
sudo chgrp _graphite /var/lib/graphite/graphite.db

##don't forget to modify the secret key in /etc/graphite/local_settings.py

sudo ln -s /usr/bin/nodejs /usr/bin/node

sudo carbon-cache --config=/etc/carbon/carbon.conf start
sudo npm install hashring
sudo apt-get install statsd

sudo cp /usr/share/graphite-web/apache2-graphite.conf  /etc/apache2/sites-available/graphite-web.conf
a2ensite graphite-web.conf
a2dissite 000-default
sudo service apache2 reload

nodejs /usr/share/statsd/stats.js  /etc/statsd/localConfig.js
