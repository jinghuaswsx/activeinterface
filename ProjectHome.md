# What is ActiveInterface #

ActiveInterface is a library that gives the user a upper abstraction level of JMS and all concepts related with it. ActiveInterface wrapper ActiveMQ-CPP (link) and it is developed in C++.

# Introduction #

When I started to develope code using JMS, it was a new technology for me. I started with the first Hello World of ACTIVEMQ-CPP and I thought that it was an very upper level API, that gives you a easy way to distribute your content. And it's true.
<p>
If you are thinking in only one producer and one consumer that sends/receive messages probably you are thinking that you dont need to use this wrapper, and its probably true. But there are some goals that this wrapper do for you.<br>
<br>
<h1>Goals</h1>

The main goals are:<br>
<br>
<ul><li><b>Easy way to create complex JMS producer/consumer networks:</b> ActiveInterface provides you a simple API to distribute your JMS networks over three concepts:<br>
<ul><li><b>Connection:</b> This connection wrappers a real producer/consumer.<br>
</li><li><b>Link:</b> Link is a middle layer that allows the user to define the default properties that are sent in all messages that are producer throw this link.<br>
</li><li><b>Service:</b> Service is the upper side and the way to send data in ActiveInterface.</li></ul></li></ul>

<blockquote>ActiveInterface group connections allowing you to send the same message to different locations. Also, ActiveInterface allows you to define the default properties that are sent with this message (through link identifier).<br>
<p>
For example, if you need to send a message to a group of persons, and this message should have different properties for each person, ActiveInterface gives you an API (link to api) or a XML configuration file to do this and so much more (link to explain architecture section).</blockquote>

<pre><code>         void newProducer (...);<br>
         void newConsumer (...);<br>
         void newLink (...);<br>
         bool destroyLink (std::string &amp;linkId) throw (ActiveException);<br>
         bool destroyService (std::string &amp;serviceId) throw (ActiveException);<br>
         void getConnsByDestination (std::string &amp;destination, std::list&lt; ActiveConnection * &gt; &amp;connectionListR) throw (ActiveException)<br>
</code></pre>

<ul><li><b>Creating/Modifying/Delete architecture in real time:</b> ActiveInterface provides you a easy way to delete, modify or create new connections, links or services in real time.<br>
<blockquote>If you need to redirect data between one destination to other, or if you want to be allowed to modify dinamically your JMS network based in events, ActiveInterface gives you a way to do it.<br>
</blockquote><blockquote><img src='http://i.picasion.com/pic34/f9d6c34dedd584322d0c559408bb14b6.gif' /></blockquote></li></ul>

<blockquote>As you can see in the figure above, ActiveInterface provides an API that allow you to create new connections (producer/consumers) and associate it to the architecture.<br>
</blockquote><ul><li><b>Initialization by XML:</b> ActiveInterface provides you to the possibility to initialize your JMS network using a XML file. This XML describes the topology of your JMS network.</li></ul>

<pre><code>           &lt;?xml version="1.0" ?&gt;<br>
           &lt;connectionslist&gt;<br>
                &lt;connection<br>
                  id="producer1"<br>
                  ipbroker="failover://(tcp://localhost:61616?                 connection.useAsyncSend=true)"<br>
                  type=1<br>
                  destination="alarms"<br>
                  persistent=0<br>
                  topic=0<br>
                /&gt;<br>
                &lt;connection<br>
                  id="consumer1"<br>
                  ipbroker="failover://(tcp://localhost:61616)"<br>
                  type=0<br>
                  destination="alarms"<br>
                  topic=0<br>
                  selector=""<br>
                  durable=0<br>
                 /&gt;<br>
            &lt;/connectionslist&gt;<br>
<br>
            &lt;linkslist&gt;<br>
                &lt;link id="link1" name="alarmas" connectionid="producer1"&gt;<br>
                &lt;properties&gt;<br>
                        &lt;property name="defaultIntProperty" type=0 value=2 /&gt;<br>
                        &lt;property name="defaultStringProperty" type=2 value="all2" /&gt;<br>
                &lt;/properties&gt;<br>
                &lt;/link&gt;<br>
            &lt;/linkslist&gt;<br>
<br>
            &lt;serviceslist&gt;<br>
               &lt;service id=1&gt;<br>
                 &lt;link id=1 /&gt;<br>
               &lt;/service&gt;<br>
            &lt;/serviceslist&gt;<br>
</code></pre>

<blockquote>This XML file defines a simple producer and consumer of a queue <i>alarms</i>. As you can see, you have to define all concepts related to a JMS connection (durability, persistent, type of connection, URI or destination) but you have to code nothing. <b>Only one person with the JMS <i>know how</i> should be enough to use it, distribute or balance messages in a JMS network.</b>
</blockquote><blockquote>ActiveInterface abstracts you of some concepts like SSL (you will only add the path to public key to XML or by API), message concepts (time to live, priorities, parameter, properties), response messages to petition, etc., but ActiveInterface was developed to don't restrict the interactuation from the user with broker or to native ActiveMQ-CPP API (you can use all connection parameters directly in the ipbroker).<br>
</blockquote><ul><li><b>Multi type messages:</b> One of the better things of ActiveInterface is that allow the user a simple way to send multitype messages. ActiveInterface wrapper a native JMS message and gives you a way to insert different types (int, real, strings or bytes) and a way to take it, as the same way that a Hash Map. This data in AciveInterface is known as <i>parameters</i>.<br>
<blockquote>Also, ActiveInterface allows you to insert properties identifying it by a key (as parameters describes above).<br>
<pre><code>      void   insertStringParameter (std::string &amp;key, std::string &amp;value)<br>
      void   insertBytesParameter (std::string &amp;key, std::vector&lt; unsigned char &gt; &amp;value)<br>
      void   insertStringProperty (std::string &amp;key, const std::string &amp;value)<br>
</code></pre></blockquote></li></ul>

<blockquote>This methods are described in the API (here). This would be good for you if you want to dont take care of the protocol and you want to insert and get the information in a simple way. Also, parameters and properties could be taken iteratively, and will allow you to send different types of messages and take dinamically if exists or not.</blockquote>

<ul><li><b>Persistence:</b> One of the things that could complicate a develop using JMS is the possibility to loss messages. With ActiveMQ-CPP if the broker is down, the execution thread is going to be blocked (for forever or only by seconds if you have defined maxRequest in URI parameters). If you are waiting forever to send a message (because each message is important for you), you could have some problems:<br>
<ul><li>If your application stops while are blocked in sends, this message is going to be lost (yes is only one).<br>
</li><li>If your application is blocked and you are not using threads, all your application is going to be blocked.<br>
</li><li>If your application is using threads (one thread per send) you could exceed the number of threads allowed by the operating system.<br>
</li></ul></li></ul><blockquote>For this problems, ActiveInterface provides you a internal queue and a persistent system that you could manipulate by your own. All messages are serialized into a persistent file, and enqueued into a internal queue that acts like a buffer of messages.<p>
This tool allows your application to hold down a very high peaks of messages and resending when will be possible. The throughput of the library is more than 1000 messages/second in normal mode and more than 200 messages/second in persistence mode. <b>This stats depends of the hosts in where you are making tests, but, tests made in the same machine using native ActiveMQ-CPP code and using ActiveInterface shows that the throughput are very similar.</b></blockquote>

<ul><li><b>Multiplatform & thread-safe:</b> ActiveInterface is a library developed and tested in a multiplatform environment (Windows and Linux) and firstly prepared to be used in a multithread application.</li></ul>

<h1>Conclusion</h1>

ActiveInterface is a library that could help the user to use JMS in a very short-time, and could allow the people to create network application based in JMS. ActiveInterface is developed to be used in client or server part.<br>
You can start learning how to use it in Getting started wiki page.