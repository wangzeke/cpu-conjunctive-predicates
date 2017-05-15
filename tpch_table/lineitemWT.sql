
 select l_orderkey, l_partkey, l_suppkey,
        l_linenumber, l_quantity, l_extendedprice, l_discount, l_tax, l_returnflag, l_linestatus, l_shipdate, l_commitdate, l_receiptdate, l_shipmode,
        o_orderstatus, o_totalprice, o_orderdate, o_orderpriority, o_shippriority, 
		c_custkey, c_name, c_phone, c_acctbal, c_nationkey, c_mktsegment,
		n1.n_name AS n1_name, n1.n_regionkey AS n1_regionkey, r1.r_name AS r1_name, 
        ps_availqty, ps_supplycost,    
        p_name, p_brand, p_type, p_size, p_retailprice,   
		s_name, s_acctbal, s_phone, s_nationkey,   
		n2.n_name AS n2_name, n2.n_regionkey AS n2_regionkey,  r2.r_name  AS r2_name
 from lineitem
 
  FULL OUTER JOIN ( 
   orders FULL OUTER JOIN(
    customer FULL OUTER JOIN (
      nation n1 FULL OUTER JOIN region r1 ON n1.n_regionkey = r1.r_regionkey
    )ON c_nationkey = n1.n_nationkey
   ) ON o_custkey = c_custkey  
  ) ON l_orderkey = o_orderkey

  FULL OUTER JOIN (
    partsupp 

      FULL OUTER JOIN part ON ps_partkey = p_partkey
      FULL OUTER JOIN (
        supplier FULL OUTER JOIN 
	      ( nation n2 FULL OUTER JOIN region r2 ON n2.n_regionkey = r2.r_regionkey
          )ON s_nationkey = n2.n_nationkey 
      ) ON ps_suppkey = s_suppkey
        
  )ON l_partkey = ps_partkey AND l_suppkey = ps_suppkey; 