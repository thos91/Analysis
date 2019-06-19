=================
Detector Topology
=================


.. figure:: ../images/Topology.png
            :width: 600px

As you can see from the picture above, for each DIF, the number of ASU chips is
not fixed. For example the WAGASCI DIFs have 20 chips each, while the SideMRD
DIFs only 3. For this reason it is not possible to just define a single integer
to denote the number of chips: we need to use a map that to each DIF assigns the
number of chips (and channels).

Moreover, the number of channels for each chip may vary. In the final setup this
number is fixed to 32 channels per chip but this may be different during
bench-testing.

wgTopology class
================

To overcome this difficulty I created a class called `wgTopology`. This class is
used to manage the number of DIFs, ASU (SPIROC2D) chips and channels. When an
object of the `wgTopology` class is constructed a map of type `TopologyMapDif`
is populated.

.. code-block:: cpp
                
                typedef std::map<string, std::map< string, unsigned>> TopologyMapDif;

The map can be accessed using the `wgTopology::map` member.


The constructor of this class accept a string and and a enum. The enum is just a
binary enum that select if the source type.

- `TopologySourceType::xml_file`

  The input value is the name of the Pyrame configuration file to read. The
  number of DIFs, ASUs and channels is retrieved from the configuration
  file. Then the `wgTopology::map` member of `TopologyMapDif` type is populated.

- `TopologySourceType::json_string`

  The `wgTopology::map` member is directly populated parsing the input JSON string.
  Here is an example JSON string:

  ```
  {"1":{"1":32,"2":32,"3":32},"2":{"1":32,"2":32,"3":32}}
  ```

Usage example
-------------

.. code-block:: cpp
   
                std::string xml_config_file("/home/neo/Code/WAGASCI/Configs/wagasci_config_6asu.xml");
                Topology topol(xml_config_file);
                std::cout << "Number of channels of DIF[1] ASU[2] = " << topol.map["1"]["2"] << std::endl;
                std::cout << "Number of chips of DIF[3] = " << topol.map["3"].size() << std::endl;
                std::cout << "Number of DIFs = " << topol.n_difs << std::endl;

Simple, isn't it?
