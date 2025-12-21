<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.11.2" name="part_3" tilewidth="32" tileheight="32" tilecount="84" columns="7">
 <image source="../Cute_Fantasy_Free/Outdoor decoration/Outdoor_Decor_32x32.png" width="224" height="384"/>
 <tile id="18">
  <properties>
   <property name="allow_pick" type="bool" value="true"/>
   <property name="base" value="plants"/>
   <property name="count_max" type="int" value="2"/>
   <property name="count_min" type="int" value="1"/>
   <property name="name" value="carrot_plant"/>
   <property name="pickup_object" value="carrot"/>
   <property name="probability" type="float" value="0.4"/>
   <property name="type" value="wild_plants"/>
  </properties>
  <objectgroup draworder="index" id="2">
   <object id="1" x="0.119963" y="0" width="31.0881" height="32.2321"/>
  </objectgroup>
 </tile>
 <tile id="19">
  <properties>
   <property name="allow_pick" type="bool" value="true"/>
   <property name="base" value="plants"/>
   <property name="count_max" type="int" value="3"/>
   <property name="count_min" type="int" value="1"/>
   <property name="name" value="bean_plant"/>
   <property name="pickup_object" value="bean"/>
   <property name="probability" type="float" value="0.4"/>
   <property name="type" value="wild_plants"/>
  </properties>
  <objectgroup draworder="index" id="2">
   <object id="1" x="4" y="0" width="26" height="28"/>
  </objectgroup>
 </tile>
 <tile id="21">
  <objectgroup draworder="index" id="2">
   <object id="1" x="2" y="0" width="30" height="32"/>
  </objectgroup>
 </tile>
 <tile id="24">
  <properties>
   <property name="HP" type="int" value="30"/>
   <property name="base" value="build"/>
   <property name="defense" type="int" value="5"/>
   <property name="drop1_Probability" type="float" value="0.75"/>
   <property name="drop_max" type="float" value="3"/>
   <property name="drop_type" value="&quot;stone&quot;"/>
   <property name="exp_max" type="int" value="12"/>
   <property name="exp_min" type="int" value="5"/>
   <property name="gold_max" type="int" value="30"/>
   <property name="gold_min" type="int" value="10"/>
   <property name="name" value="stone_build2"/>
   <property name="type" value="stone_build"/>
  </properties>
 </tile>
</tileset>
