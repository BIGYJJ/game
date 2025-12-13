<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.11.2" name="tree" tilewidth="64" tileheight="64" tilecount="3" columns="0">
 <grid orientation="orthogonal" width="1" height="1"/>
 <properties>
  <property name="name" value="tree1"/>
 </properties>
 <tile id="0">
  <properties>
   <property name="HP" type="int" value="30"/>
   <property name="defense" type="int" value="5"/>
   <property name="drop1_Probability" type="float" value="0.75"/>
   <property name="drop2_Probability" type="float" value="0.5"/>
   <property name="drop3_Probability" type="float" value="0.4"/>
   <property name="drop_max" type="float" value="3"/>
   <property name="drop_type" value="&quot;wood&quot;,&quot;seed&quot;,&quot;stick&quot;"/>
   <property name="name" value="tree1"/>
   <property name="type" value="tree"/>
  </properties>
  <image source="tree/tree.png" width="64" height="64"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="2" y="0" width="61" height="64"/>
  </objectgroup>
 </tile>
 <tile id="1">
  <properties>
   <property name="HP" type="int" value="40"/>
   <property name="defense" type="int" value="5"/>
   <property name="drop1_Probability" type="float" value="0.75"/>
   <property name="drop2_Probability" type="float" value="0.5"/>
   <property name="drop3_Probability" type="float" value="0.4"/>
   <property name="drop4_Probability" type="float" value="0.3"/>
   <property name="drop_max" type="float" value="3"/>
   <property name="drop_type" value="&quot;wood&quot;,&quot;seed&quot;,&quot;stick&quot;,&quot;apple&quot;"/>
   <property name="name" value="apple_tree"/>
   <property name="type" value="tree"/>
  </properties>
  <image source="tree/apple_tree.png" width="64" height="64"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="2" y="0" width="61" height="64"/>
  </objectgroup>
 </tile>
 <tile id="2">
  <properties>
   <property name="HP" type="int" value="100"/>
   <property name="defense" type="int" value="5"/>
   <property name="drop1_Probability" type="float" value="0.75"/>
   <property name="drop2_Probability" type="float" value="0.5"/>
   <property name="drop3_Probability" type="float" value="0.4"/>
   <property name="drop4_Probability" type="float" value="0.3"/>
   <property name="drop_max" type="float" value="3"/>
   <property name="drop_type" value="&quot;wood&quot;,&quot;seed&quot;,&quot;stick&quot;,&quot;cherry&quot;"/>
   <property name="name" value="cherry_tree"/>
   <property name="type" value="tree"/>
  </properties>
  <image source="tree/cherry_tree.png" width="64" height="64"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="2" y="0" width="61" height="64"/>
  </objectgroup>
 </tile>
</tileset>
