# activeinterface
Automatically exported from code.google.com/p/activeinterface

#What is ActiveInterface
ActiveInterface is a library that gives the user a upper abstraction level of JMS and all concepts related with it. ActiveInterface wrapper ActiveMQ-CPP (link) and it is developed in C++.

$Introduction
When I started to develope code using JMS, it was a new technology for me. I started with the first Hello World of ACTIVEMQ-CPP and I thought that it was an very upper level API, that gives you a easy way to distribute your content. And it's true.

If you are thinking in only one producer and one consumer that sends/receive messages probably you are thinking that you dont need to use this wrapper, and its probably true. But there are some goals that this wrapper do for you.

#Goals
The main goals are:

Easy way to create complex JMS producer/consumer networks: ActiveInterface provides you a simple API to distribute your JMS networks over three concepts:
Connection: This connection wrappers a real producer/consumer.
Link: Link is a middle layer that allows the user to define the default properties that are sent in all messages that are producer throw this link.
Service: Service is the upper side and the way to send data in ActiveInterface.
ActiveInterface group connections allowing you to send the same message to different locations. Also, ActiveInterface allows you to define the default properties that are sent with this message (through link identifier).
For example, if you need to send a message to a group of persons, and this message should have different properties for each person, ActiveInterface gives you an API (link to api) or a XML configuration file to do this and so much more (link to explain architecture section).
         void newProducer (...);
         void newConsumer (...);
         void newLink (...);
         bool destroyLink (std::string &linkId) throw (ActiveException);
         bool destroyService (std::string &serviceId) throw (ActiveException);
         void getConnsByDestination (std::string &destination, std::list< ActiveConnection * > &connectionListR) throw (ActiveException)
