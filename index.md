# Applied Resiliency for More Trustworthy Grid Operation (ARMORE)


*(Scheduled for beta release Fall 2015)*

## ARMORE

ARMORE is being developed to be an open-source software solution that will aid asset owners by increasing visibility, securing communications, and inspecting ICS communications for behavior that is not intended.

* Identify
	* ARMORE aids in detecting assets and can provide some passively learned metadata regarding those assets.
* Detect
	* ARMORE inspects traffic looking for unintended communications or undesirable behavior.
* Protect
	* ARMORE can alarm or enforce a defined policy.
* Respond
	* ARMORE can be utilized to feed into other decision systems, aid visualization of situational awareness, or if configured, serve as a forensic tool to investigate suspicious traffic.


## More Details

ARMORE combines inspection with optional actions, including alarming or enforcement when communications do not comply with an instantiated ARMORE policy. This can work for many standard protocols, but ARMORE is focused on Industrial Control Systems and currently targets DNP3 and Modbus for its primary policies.

To aid visibility, ARMORE tracks ICS communications to gather statistics on communications patterns, conversation pairs, and details about what the communications is about. For example, ARMORE can tell you the frequency that two hosts communicate, what protocols they speak, what functions they call and what the targets are of those functions. ARMORE could even be set to look at the values and determine if they are in range, or hand those values off for additional computation. Computation hooks could be leveraged to feed the results into state estimation functions, calculate averages in a time window, look for value trends, or many other features.

ARMORE will also be capable of encapsulating and encrypting legacy communications and resiliently exchanging this information among ARMORE nodes. It does so with an abstracted middleware layer that encapsulates communications from one point to others. For the initial implementation, ARMORE utilizes a secure transport mode of operation with ZeroMQ.

Bro, an open source network analysis platform, is the core packet engine that is leveraged by ARMORE to inspect the network packets and reason about them. Bro enables ARMORE to conduct a semantic analysis of network traffic in process control and other networks and then decide what to do. Because of Bro, ARMORE can collect statistics, inspect relevant traffic, and call out to apply policies to that traffic to help secure critical infrastructure from attackers.

Combine this reasoning with other components of ARMORE and a node can also securely communicating both known and unknown protocols to their intended destination with increased reliability and resiliency.

## Deployment and Licensing

ARMORE is open source and intended to create an easy path to adoption. One of the key ways this is accomplished is by architecting ARMORE such that it supports multiple modes of deployment. These modes can be summarized in two high-level categories, passive and active.

* Passive
	* Span port configuration that allows the system to inspect traffic, but not modify. This could include limited policy support with alarming as the only (initially) available action.
* Active
	* Active mode involves placing the ARMORE node inline of communications. Under active mode, there are two primary modes of operation, transparent and proxied.
		* Transparent
			* ARMORE operates as a transparent bridge, not manipulating the traffic flowing through it in any way except to optionally enforce policy. In this mode, ARMORE can also do everything that passive mode can do, but gains actual enforcement capabilities.
		* Proxied
			* ARMORE operates as an encapsulating bridge, taking the raw unmodified packets and packaging them into a middleware communication to be safely transferred to their destination before being de-encapsulated and put back onto the wire. This allows the middleware layer to encrypt these communications, provide resilient communications, or add additional metadata around those communications. In this mode, ARMORE can do everything transparent mode can, but adds the security layer of encrypted communications and added resiliency.


## Project Details

This effort is funded by the Department of Energy Office of Electricity and organized by the Grid Protection Alliance (GPA). The University of Illinois is the technical lead for the effort with Pacific Northwest National Labs providing input and external assessment of the effort.

## Contact

For more information, contact Tim Yardley (<yardley@illinois.edu>).
