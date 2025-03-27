"""
DNS Resolver Implementation for CS425 Assignment 2
Supports both iterative and recursive resolution modes.

"""

import dns.message
import dns.query
import dns.rdatatype
import dns.resolver
import time
import random
import sys


# Root DNS servers with their IP addresses and symbolic names
# These are the starting points for iterative DNS resolution

ROOT_SERVERS = {
    "198.41.0.4": "Root (a.root-servers.net)",
    "199.9.14.201": "Root (b.root-servers.net)",
    "192.33.4.12": "Root (c.root-servers.net)",
    "199.7.91.13": "Root (d.root-servers.net)",
    "192.203.230.10": "Root (e.root-servers.net)"
}


# Timeout for DNS queries in seconds

TIMEOUT = 3


# ---------------------------------------------------------------
# Core DNS Functions
# ---------------------------------------------------------------


# Sending a DNS query to a specified server for a domain's A record.

def send_dns_query(server, domain):
    

    """
    Parameters:
    server (str): IP address of the DNS server to query
    domain (str): Domain name to resolve
    Returns:
    dns.message.Message: DNS response object or None if query fails
    
    """

    try:

        # Construct DNS query for A record

        query = dns.message.make_query(domain, dns.rdatatype.A)

        # UDP preferred for DNS queries (faster, smaller packets)
        # Send query over UDP with timeout

        response = dns.query.udp(query, server, timeout=TIMEOUT)         
        return response
        
    except dns.exception.Timeout:

        # DNS-over-UDP specific failure (No response received within timeout)
        print(f"[TIMEOUT] Query to {server} timed out after {TIMEOUT}s")
    
    except Exception as e:

        # Catch-all for unexpected failures
        print(f"[ERROR] Query to {server} failed: {str(e)}")
    
    return None

# Processing DNS response to extract and resolve next nameservers.

def extract_next_nameservers(response):
   
    """
    Parameters:
    response (dns.message.Message): DNS response object
    
    Returns:
    list: IP addresses of authoritative nameservers
    
    """
    ns_ips = []
    ns_names = []

    # Process authority section for NS records (NS records define authoritative servers)

    if response.authority:

        for rrset in response.authority:  

            if rrset.rdtype == dns.rdatatype.NS:

                for rr in rrset:

                    # Remove trailing dot from FQDN

                    ns_name = rr.to_text().rstrip('.')
                    ns_names.append(ns_name)
                    print(f"Extracted NS hostname: {ns_name}")


    # Process additional section for glue records (IP addresses)

    if not ns_names and response.additional:

        for rrset in response.additional:
        
            if rrset.rdtype == dns.rdatatype.A:
        
                for item in rrset:
        
                    ns_ips.append(item.address)
                    print(f"Found glue record: {rrset.name} -> {item.address}")


    # Resolve NS names to IP addresses

    for ns_name in ns_names:

        try:
        
            answer = dns.resolver.resolve(ns_name, 'A')
        
            for rdata in answer:
        
                ns_ips.append(rdata.address)
                print(f"Resolved {ns_name} -> {rdata.address}")
                
        except dns.resolver.NoAnswer:

            # Server responded but no A/NS records found
            print(f"[WARNING] No A record found for {ns_name}")
        
        except dns.resolver.NXDOMAIN:

            # Domain doesn't exist in DNS registry (permanent error)    
            print(f"[ERROR] Invalid NS hostname {ns_name}")
        
        except dns.resolver.Timeout:
        
            # Transient network/DNS server issue - could retry    
            print(f"[TIMEOUT] Resolution timed out for {ns_name}")
        
        except Exception as e:

           # Catch-all for unexpected errors    
            print(f"[ERROR] Failed to resolve {ns_name}: {str(e)}")

    return ns_ips

# Performing iterative DNS resolution following the hierarchy:
# Root Server → TLD Server → Authoritative Server

def iterative_dns_lookup(domain):
    
    """
    Parameters:
    domain (str): Domain name to resolve

    """
    
    print(f"[Iterative DNS Lookup] Resolving {domain}")
    
    # Start with root servers and initial resolution stage

    next_ns_list = list(ROOT_SERVERS.keys())
    stage = "ROOT"
    
    while next_ns_list:

        # Randomize server selection for load balancing

        random.shuffle(next_ns_list)
        ns_ip = next_ns_list.pop()         # Get last server from randomized list
        
        # Send DNS query and handle response

        response = send_dns_query(ns_ip, domain)
        
        if not response:

            print(f"[ERROR] Query failed for {stage} server {ns_ip}")
            continue
            
        print(f"[DEBUG] Querying {stage} server ({ns_ip}) - SUCCESS")


        # Check for direct answer in response

        if response.answer:
           
            for answer in response.answer:
           
                for item in answer:
           
                    if item.rdtype == dns.rdatatype.A:
           
                        print(f"[SUCCESS] {domain} -> {item.address}")
                        return


        # Update nameserver list and progress through stages

        next_ns_list = extract_next_nameservers(response)
        

        # Advance resolution stage

        if stage == "ROOT":
            stage = "TLD"          # After root servers, query Top-Level Domain servers
        
        elif stage == "TLD":
            stage = "AUTH"
            
    print("[ERROR] Resolution failed: No more nameservers to query")


# Performing recursive DNS resolution using public resolvers.

def recursive_dns_lookup(domain):

    """
    Parameters:
    domain (str): Domain name to resolve

    """

    print(f"[Recursive DNS Lookup] Resolving {domain}")

    try:

        # Fetch NS records (nameservers)

        ns_answer = dns.resolver.resolve(domain, 'NS')
       
        for rdata in ns_answer:
        
            print(f"[SUCCESS] {domain} -> {rdata.target}")
        
        # Fetch A record (final IP address)

        a_answer = dns.resolver.resolve(domain, 'A')
        
        for rdata in a_answer:
        
            print(f"[SUCCESS] {domain} -> {rdata.address}")
            
    except dns.resolver.NXDOMAIN:

        # Domain doesn't exist in DNS registry (permanent error)
        print(f"[ERROR] Domain {domain} does not exist")

    except dns.resolver.Timeout:

        # Transient network/DNS server issue - could retry
        print("[ERROR] DNS resolution timed out")

    except dns.resolver.NoAnswer:

        # Server responded but no A/NS records found
        print(f"[ERROR] No records found for {domain}")

    except Exception as e:

        # Catch-all for unexpected errors like network issues
        print(f"[ERROR] Unexpected failure: {str(e)}")



# ---------------------------------------------------------------
# Main Execution
# ---------------------------------------------------------------

if __name__ == "__main__":

    """
    Command-line interface handler
    Usage:
    python3 dnsresolver.py [iterative|recursive] [domain]
    
    """
    
    # Argument validation

    if len(sys.argv) != 3 or sys.argv[1] not in {"iterative", "recursive"}:
       
        print("Invalid arguments!")
        print("Usage: python3 dnsresolver.py [iterative|recursive] [domain]")
        print("Example: python3 dnsresolver.py iterative google.com")
       
        sys.exit(1)

    # Parse arguments

    mode = sys.argv[1]
    domain = sys.argv[2]
    
    try:

        start_time = time.time()
        
        if mode == "iterative":
            iterative_dns_lookup(domain)
        
        else:
            recursive_dns_lookup(domain)
            
        print(f"Time taken: {time.time() - start_time:.3f} seconds")
        
    except KeyboardInterrupt:

        print("\n[INFO] Operation cancelled by user")
        
        sys.exit(0)