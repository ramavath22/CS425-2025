# DNS Resolver 

This project implements a DNS resolver that supports both iterative and recursive DNS lookups.

## Overview

The DNS resolver script allows you to:
- Perform iterative DNS resolution by querying root servers, TLD servers, and authoritative servers.
- Perform recursive DNS resolution using the python dns library's DNS resolver.

## Features
- Supports both iterative and recursive DNS lookups
- Displays detailed logs of the resolution process
- Provides time taken for DNS resolution

## Usage
Ensure denpendencies are installed:
bash
pip install -r requirements.txt

Run the script with the following command:

```bash
python dnsresolver.py <mode> <domain>
```
### Parameters
- `<mode>`: iterative or recursive
- `<domain>`: The domain name you want to resolve

### Examples

#### Iterative DNS Lookup

```bash
python3 dnsresolver.py iterative google.com

[ Iterative DNS Lookup ] Resolving google . com
[ DEBUG ] Querying ROOT server (198.41.0.4) - SUCCESS
Extracted NS hostname : l.gtld-servers.net.
Extracted NS hostname : j.gtld-servers.net.
Extracted NS hostname : h.gtld-servers.net.
Extracted NS hostname : d.gtld-servers.net.
Extracted NS hostname : b.gtld-servers.net.
Extracted NS hostname : f.gtld-servers.net.
Extracted NS hostname : k.gtld-servers.net.
Extracted NS hostname : m.gtld-servers.net.
Extracted NS hostname : i.gtld-servers.net.
Extracted NS hostname : g.gtld-servers.net.
Extracted NS hostname : a.gtld-servers.net.
Extracted NS hostname : c.gtld-servers.net.
Extracted NS hostname : e.gtld-servers.net.
Resolved l.gtld-servers.net. to 192.41.162.30
Resolved j.gtld-servers.net. to 192.48.79.30
Resolved h.gtld-servers.net. to 192.54.112.30
Resolved d.gtld-servers.net. to 192.31.80.30
Resolved b.gtld-servers.net. to 192.33.14.30
Resolved f.gtld-servers.net. to 192.35.51.30
Resolved k.gtld-servers.net. to 192.52.178.30
Resolved m.gtld-servers.net. to 192.55.83.30
Resolved i.gtld-servers.net. to 192.43.172.30
Resolved g.gtld-servers.net. to 192.42.93.30
Resolved a.gtld-servers.net. to 192.5.6.30
Resolved c.gtld-servers.net. to 192.26.92.30
Resolved e.gtld-servers.net. to 192.12.94.30
[ DEBUG ] Querying TLD server (192.41.162.30) - SUCCESS
Extracted NS hostname : ns2.google.com.
Extracted NS hostname : ns1.google.com.
Extracted NS hostname : ns3.google.com.
Extracted NS hostname : ns4.google.com.
Resolved ns2.google.com. to 216.239.34.10
Resolved ns1.google.com. to 216.239.32.10
Resolved ns3.google.com. to 216.239.36.10
Resolved ns4.google.com. to 216.239.38.10
[ DEBUG ] Querying AUTH server (216.239.34.10) - SUCCESS
[ SUCCESS ] google . com -> 142.250.194.78
Time taken : 0.654 seconds

```

#### Recursive DNS Lookup

```bash

python3 dnsresolver.py recursive google.com

[ Recursive DNS Lookup ] Resolving google.com
[ SUCCESS ] google.com -> ns4.google.com.
[ SUCCESS ] google.com -> ns3.google.com.
[ SUCCESS ] google.com -> ns2.google.com.
[ SUCCESS ] google.com -> ns1.google.com.
[ SUCCESS ] google.com -> 172.217.167.206
Time taken : 0.016 seconds

```

## Explanation of TODO Sections

### 1. Sending a DNS Query (send_dns_query function)

#### TODO: Implement UDP-based DNS Query Handling


In this section, the script constructs a DNS query for an A record and sends it via UDP using the dns.query.udp() method.
#### Explanation:


1. The function creates a DNS query for an A record using dns.message.make_query().
2. The query is sent over UDP, which is preferred for DNS queries due to its low latency and reduced overhead.
3. EDNS (Extension Mechanisms for DNS) is enabled with a buffer size of 4096 bytes to support larger DNS responses.
    - This ensures the script receives all nameserver IP addresses in the response.additional section.
4. The function implements error handling:
    - If the query succeeds, it returns the DNS response object.
    - If the query times out, an error message is printed, and the function returns None.
    - If any unexpected errors occur, they are caught and logged.


### 2. Extracting Next Nameservers (extract_next_nameservers function)

#### TODO: Resolve the Extracted NS Hostnames to IP Addresses

This section extracts NS records from the authority section of a DNS response and attempts to resolve them into IP addresses

#### Explanation:
1. The function first extracts NS records (nameserver hostnames) from the authority section of the DNS response.
2. If the additional section contains glue records (pre-resolved IP addresses for the NS records), these are used immediately.
3. If no glue records are available, the script performs a separate query to resolve the NS hostnames to IP addresses.
4. The function includes error handling for:
   - NXDOMAIN: If an NS hostname does not exist.
   - NoAnswer: If an A record is missing for an NS hostname.
   - Timeouts: If the query takes too long to resolve an NS hostname.

### 3. Handling DNS Resolution Stages (iterative_dns_lookup function)

#### TODO: Implement Iterative Resolution Across Hierarchical DNS Stages

This function performs an iterative DNS resolution process, moving through different levels of the DNS hierarchy:

#### Explanation:
1. The function starts with a randomized list of root DNS servers (for load balancing).
2. It queries the root servers first, then extracts the TLD (Top-Level Domain) servers from the response.
3. The script progresses through the resolution stages:
   - ROOT stage: Queries root servers.
   - TLD stage: Queries Top-Level Domain servers (e.g., .com, .org).
   - AUTH stage: Queries authoritative nameservers for the final resolution.
4. The function continues iterating through available nameservers until an answer is found or no more nameservers are available.
5. The function includes error handling for:
   - Failed queries: If a query does not return a response, the function tries the next available nameserver.
   - Loop termination: If all nameservers have been queried and no valid response is obtained, the resolution process stops.



### 4. Recursive DNS Lookup (recursive_dns_lookup function)

#### TODO: Implement Recursive Resolution Using System's Recursive Resolver
This function leverages the system’s recursive DNS resolver (e.g., Google DNS, ISP resolver) to fetch DNS records automatically.

#### Explanation:
1. The function first queries NS records to find the nameservers responsible for the domain.
2. Then, it queries for the A record to resolve the domain to an IP address.
3. The function uses dns.resolver.resolve(), which allows the recursive resolver to perform the lookup instead of handling it manually.
4. The function includes error handling for:
   - NXDOMAIN: The domain does not exist in DNS.
   - Timeouts: The query took too long.
   - NoAnswer: No relevant records were found.



## Challenges Faced
-By default, the UDP response size is limited to 512 bytes, which is often insufficient for retrieving all nameserver records. This limitation was addressed by enabling EDNS (Extension Mechanisms for DNS) with a buffer size of 4096 bytes (see send_dns_query() function). With this enhancement, the script can successfully retrieve and resolve nameserver IP addresses from the additional section of the response (response.additional).


## Contribution Breakdown
| Member |Roll number| Contribution (%) | Tasks |
|--------|------|----------|-------|
|Ramavath Dinesh Naik   |  220866   |35%     | Code  |
|L.Sree Sahithi  |   220577  |32.5%     | Comments and README |
|Gowtham Chand    |   220313  |32.5%     | README |

## References
- DNS python [manual](https://dnspython.readthedocs.io/en/latest/manual.html)

## Declaration
We, (*Dinesh, Sahithi and Gowtham Chand*) declare that this assignment was completed independently without plagiarism. Any external sources used are appropriately referenced.