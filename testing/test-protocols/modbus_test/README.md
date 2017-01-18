###############################################################################  
# Running Modbus Test Scripts  
###############################################################################  

# Installing the environment  

1. ```sudo apt-get install python-pip python-dev```  
2. ```sudo pip install virtualenv```  

# Directory where modbus scripts are located  
3. cd modbus_directory  

# Create a virtual environment  
4. virtualenv venv  

# Activate the Virtual Environment  
5. source venv/bin/activate  

# Install python modbus into virtual environment  

pip version of pymodbus is old (1.2.0) which doesn't have socket  
binding for client. We need >= (1.3.0)  

Instead do:  

6. git clone pymodbus https://github.com/bashwork/pymodbus  
7. python setup.py install  
8. pip install -U netifaces  
9. pip install -U ipaddress  
10. pip install -U netaddr  
11. pip install -U PyCrypto  

#  We use ./venv/bin/python since we started using virtual environments   

# For Client  
```sudo ./venv/bin/python python modbusclient.py```  

# For Server
```sudo ./venv/bin/python python modbusserver.py```  

# Traffic Generator example  
```sudo ./venv/bin/python modbusgen.py 2 2 eth0 192.168.5.0/29```  
After each run for the traffic generation scripts, it may take upwards of a few minute before the sockets are released.  
You can view the status of the sockets by doing: ```netstat -a```  

You can override the default socket timeout in linux until reboot by doing the following:  
```sudo sysctl net.ipv4.tcp_syn_retries=1```  

# To kill virtual environment  
```deactivate```  
# If you need to run the server/client on Priviliged ports, i.e. Ports under 1024 Without running in a virtual environment  
1. ```cd modbus_directory```  
2. ```sudo ./venv/bin/python ./modbusclient.py #For Client```  
3. ```sudo ./venv/bin/python ./modbusserver.py #For Server```  

### Note: Some of the Modbus functions like read/write are not implemented. Make sure to check functionality

###############################################################################  
# Virtual Network Traffic  
###############################################################################  
Due to network fast path on local interface aliases, we'll need to install dummynet and ipfw to add
jitters and other delays or latencies.  

The project can be obtained over here:  http://info.iet.unipi.it/~luigi/dummynet/  

1. You will need the kernel source to compile.  
2. From there you'll have to insert the kernel module: ```sudo insmod ./kipfw-mod/ipfw_mod.ko```  
3. You can then create named pipes for the interfaces from: ```./ipfw/ipfw```

# Example Commands to limit all traffic on the 192.168.6.0/24:  
```sudo ./ipfw add pipe 1 src-ip 192.168.6.0/24 in```  
```sudo ./ipfw pipe 1 config bw 100kbit/s queue 20 delay 10ms```  

# To remove all the rules from the pipes  
``` sudo ./ipfw flush```  

# To view all the rules on the pipes
``` sudo ./ipfw show```  



