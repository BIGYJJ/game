#include "Item.h"
#include <iostream>

// ============================================================================
// ItemDatabase 单例实现
// ============================================================================

ItemDatabase& ItemDatabase::getInstance() {
    static ItemDatabase instance;
    return instance;
}

void ItemDatabase::initialize() {
    if (initialized) return;
    
    std::cout << "[ItemDatabase] Initializing item definitions..." << std::endl;
    
    // ========================================
    // 材料类物品 (Materials)
    // 存放目录: assets/materials/
    // ========================================
    
    {
        ItemData wood;
        wood.id = "wood";
        wood.name = "木材";
        wood.description = "从树木上砍伐获得的木材，可用于建造和合成";
        wood.type = ItemType::Material;
        wood.rarity = ItemRarity::Common;
        wood.maxStack = 99;
        wood.sellPrice = 2;
        wood.buyPrice = 10;
        wood.texturePath = "assets/materials/wood.png";
        registerItem(wood);
    }
    
    {
        ItemData stick;
        stick.id = "stick";
        stick.name = "树枝";
        stick.description = "细小的树枝，可用于制作简单工具";
        stick.type = ItemType::Material;
        stick.rarity = ItemRarity::Common;
        stick.maxStack = 99;
        stick.sellPrice = 1;
        stick.buyPrice = 5;
        stick.texturePath = "assets/materials/stick.png";
        registerItem(stick);
    }
    
    {
        ItemData seed;
        seed.id = "seed";
        seed.name = "种子";
        seed.description = "可以种植的树种子。";
        seed.type = ItemType::Material;
        seed.rarity = ItemRarity::Common;
        seed.maxStack = 99;
        seed.sellPrice = 5;
        seed.buyPrice = 20;
        seed.texturePath = "assets/materials/seed.png";
        registerItem(seed);
    }
    
    {
        ItemData stone;
        stone.id = "stone";
        stone.name = "石头";
        stone.description = "普通的石头，可用于建造";
        stone.type = ItemType::Material;
        stone.rarity = ItemRarity::Common;
        stone.maxStack = 99;
        stone.sellPrice = 1;
        stone.buyPrice = 5;
        stone.texturePath = "assets/materials/stone.png";
        registerItem(stone);
    }
    
    // 兔子掉落物品
    {
        ItemData rabbitFur;
        rabbitFur.id = "rabbit_fur";
        rabbitFur.name = "兔毛";
        rabbitFur.description = "柔软的兔毛。可作为孵化兔子精元时的强化剂，提升稀有资质概率！";
        rabbitFur.type = ItemType::Material;
        rabbitFur.rarity = ItemRarity::Common;
        rabbitFur.maxStack = 99;
        rabbitFur.sellPrice = 5;
        rabbitFur.buyPrice = 15;
        rabbitFur.texturePath = "assets/materials/rabbit_fur.png";
        registerItem(rabbitFur);
    }
    
    {
        ItemData rabbitMeat;
        rabbitMeat.id = "rabbit_meat";
        rabbitMeat.name = "兔肉";
        rabbitMeat.description = "新鲜的兔肉，可以烹饪食用";
        rabbitMeat.type = ItemType::Consumable;
        rabbitMeat.rarity = ItemRarity::Uncommon;  // 优秀
        rabbitMeat.maxStack = 20;
        rabbitMeat.sellPrice = 15;
        rabbitMeat.buyPrice = 40;
        rabbitMeat.texturePath = "assets/consumables/rabbit_meat.png";
        rabbitMeat.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 30));
        registerItem(rabbitMeat);
    }
    
    // ========================================
    // 家禽掉落物品
    // ========================================
    
    // 谷物
    {
        ItemData cereal;
        cereal.id = "cereal";
        cereal.name = "谷物";
        cereal.description = "可以喂养家禽的谷物";
        cereal.type = ItemType::Material;
        cereal.rarity = ItemRarity::Common;
        cereal.maxStack = 99;
        cereal.sellPrice = 20;
        cereal.buyPrice = 50;
        cereal.texturePath = "assets/materials/cereal.png";
        registerItem(cereal);
    }
    
    // 虫子
    {
        ItemData bug;
        bug.id = "bug";
        bug.name = "虫子";
        bug.description = "小鸡喜欢吃的虫子";
        bug.type = ItemType::Material;
        bug.rarity = ItemRarity::Common;
        bug.maxStack = 99;
        bug.sellPrice = 20;
        bug.buyPrice = 50;
        bug.texturePath = "assets/materials/bug.png";
        registerItem(bug);
    }
    
    // 鸡肉
    {
        ItemData chicken;
        chicken.id = "chicken";
        chicken.name = "鸡肉";
        chicken.description = "新鲜的鸡肉";
        chicken.type = ItemType::Consumable;
        chicken.rarity = ItemRarity::Uncommon;  // 良好
        chicken.maxStack = 99;
        chicken.sellPrice = 30;
        chicken.buyPrice = 80;
        chicken.texturePath = "assets/materials/chicken.png";
        chicken.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 25));
        registerItem(chicken);
    }
    
    // 鸡蛋
    {
        ItemData egg;
        egg.id = "egg";
        egg.name = "鸡蛋";
        egg.description = "母鸡产出的鸡蛋";
        egg.type = ItemType::Material;
        egg.rarity = ItemRarity::Common;
        egg.maxStack = 99;
        egg.sellPrice = 15;
        egg.buyPrice = 40;
        egg.texturePath = "assets/materials/egg.png";
        registerItem(egg);
    }
    
    // 猪肉
    {
        ItemData pork;
        pork.id = "pork";
        pork.name = "猪肉";
        pork.description = "新鲜的猪肉";
        pork.type = ItemType::Consumable;
        pork.rarity = ItemRarity::Uncommon;  // 良好
        pork.maxStack = 99;
        pork.sellPrice = 50;
        pork.buyPrice = 120;
        pork.texturePath = "assets/materials/pork.png";
        pork.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 35));
        registerItem(pork);
    }
    
    // 猪皮
    {
        ItemData pigskin;
        pigskin.id = "pigskin";
        pigskin.name = "猪皮";
        pigskin.description = "猪的皮，可用于制作皮革制品";
        pigskin.type = ItemType::Material;
        pigskin.rarity = ItemRarity::Uncommon;  // 良好
        pigskin.maxStack = 99;
        pigskin.sellPrice = 100;
        pigskin.buyPrice = 250;
        pigskin.texturePath = "assets/materials/pigskin.png";
        registerItem(pigskin);
    }
    
    // 猪后腿
    {
        ItemData pigHindLeg;
        pigHindLeg.id = "pig_hind_leg";
        pigHindLeg.name = "猪后腿";
        pigHindLeg.description = "猪的后腿，非常珍贵的食材";
        pigHindLeg.type = ItemType::Material;
        pigHindLeg.rarity = ItemRarity::Rare;  // 优秀
        pigHindLeg.maxStack = 99;
        pigHindLeg.sellPrice = 300;
        pigHindLeg.buyPrice = 800;
        pigHindLeg.texturePath = "assets/materials/pig_hind_leg.png";
        registerItem(pigHindLeg);
    }
    
    // 骨头
    {
        ItemData bone;
        bone.id = "bone";
        bone.name = "骨头";
        bone.description = "动物的骨头，可用于制作工具";
        bone.type = ItemType::Material;
        bone.rarity = ItemRarity::Uncommon;  // 良好
        bone.maxStack = 99;
        bone.sellPrice = 120;
        bone.buyPrice = 300;
        bone.texturePath = "assets/materials/bone.png";
        registerItem(bone);
    }
    
    // 猪心
    {
        ItemData porkHeart;
        porkHeart.id = "pork_heart";
        porkHeart.name = "猪心";
        porkHeart.description = "猪的心脏，稀有的掉落物";
        porkHeart.type = ItemType::Material;
        porkHeart.rarity = ItemRarity::Rare;  // 优秀
        porkHeart.maxStack = 99;
        porkHeart.sellPrice = 360;
        porkHeart.buyPrice = 900;
        porkHeart.texturePath = "assets/materials/pork_heart.png";
        registerItem(porkHeart);
    }
    
    // 臭猪肉
    {
        ItemData stinkyPork;
        stinkyPork.id = "stinky_pork";
        stinkyPork.name = "臭猪肉";
        stinkyPork.description = "臭猪的肉，有特殊的气味";
        stinkyPork.type = ItemType::Material;
        stinkyPork.rarity = ItemRarity::Uncommon;  // 良好
        stinkyPork.maxStack = 99;
        stinkyPork.sellPrice = 150;
        stinkyPork.buyPrice = 400;
        stinkyPork.texturePath = "assets/materials/stinky_pork.png";
        registerItem(stinkyPork);
    }
    
    // 牛肉
    {
        ItemData beef;
        beef.id = "beef";
        beef.name = "牛肉";
        beef.description = "新鲜的牛肉";
        beef.type = ItemType::Consumable;
        beef.rarity = ItemRarity::Uncommon;  // 良好
        beef.maxStack = 99;
        beef.sellPrice = 180;
        beef.buyPrice = 450;
        beef.texturePath = "assets/materials/beef.png";
        beef.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 50));
        registerItem(beef);
    }
    
    // 耗牛毛
    {
        ItemData yakFur;
        yakFur.id = "yak_fur";
        yakFur.name = "耗牛毛";
        yakFur.description = "耗牛的毛发，可用于制作保暖物品";
        yakFur.type = ItemType::Material;
        yakFur.rarity = ItemRarity::Uncommon;  // 良好
        yakFur.maxStack = 99;
        yakFur.sellPrice = 200;
        yakFur.buyPrice = 500;
        yakFur.texturePath = "assets/materials/yak_fur.png";
        registerItem(yakFur);
    }
    
    // ========================================
    // 冶炼材料
    // ========================================
    
    // 生铁
    {
        ItemData pigIron;
        pigIron.id = "pig_iron";
        pigIron.name = "生铁";
        pigIron.description = "未经精炼的铁，可以进一步加工成钢铁";
        pigIron.type = ItemType::Material;
        pigIron.rarity = ItemRarity::Uncommon;  // 良好
        pigIron.maxStack = 99;
        pigIron.sellPrice = 130;
        pigIron.buyPrice = 350;
        pigIron.texturePath = "assets/materials/pig_iron.png";
        registerItem(pigIron);
    }
    
    // 钢铁
    {
        ItemData steel;
        steel.id = "steel";
        steel.name = "钢铁";
        steel.description = "精炼过的钢铁，可用于打造武器";
        steel.type = ItemType::Material;
        steel.rarity = ItemRarity::Rare;  // 优秀
        steel.maxStack = 99;
        steel.sellPrice = 200;
        steel.buyPrice = 550;
        steel.texturePath = "assets/materials/steel.png";
        registerItem(steel);
    }
    
    // 煤炭
    {
        ItemData coal;
        coal.id = "coal";
        coal.name = "煤炭";
        coal.description = "用于冶炼的燃料";
        coal.type = ItemType::Material;
        coal.rarity = ItemRarity::Common;
        coal.maxStack = 99;
        coal.sellPrice = 80;
        coal.buyPrice = 200;
        coal.texturePath = "assets/materials/coal.png";
        registerItem(coal);
    }
    
    {
        ItemData carrot;
        carrot.id = "carrot";
        carrot.name = "胡萝卜";
        carrot.description = "新鲜的胡萝卜，兔子的最爱";
        carrot.type = ItemType::Consumable;
        carrot.rarity = ItemRarity::Common;
        carrot.maxStack = 20;
        carrot.sellPrice = 3;
        carrot.buyPrice = 10;
        carrot.texturePath = "assets/consumables/carrot.png";
        carrot.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 8));
        registerItem(carrot);
    }
    
    {
        ItemData bean;
        bean.id = "bean";
        bean.name = "豆子";
        bean.description = "新鲜的豆子，可以食用或种植";
        bean.type = ItemType::Consumable;
        bean.rarity = ItemRarity::Common;
        bean.maxStack = 20;
        bean.sellPrice = 2;
        bean.buyPrice = 8;
        bean.texturePath = "assets/consumables/bean.png";
        bean.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 5));
        registerItem(bean);
    }
    
    // ========================================
    // 消耗品类物品 (Consumables)
    // 存放目录: assets/consumables/
    // ========================================
    
    {
        ItemData apple;
        apple.id = "apple";
        apple.name = "苹果";
        apple.description = "新鲜的苹果，食用后恢复少量生命值";
        apple.type = ItemType::Consumable;
        apple.rarity = ItemRarity::Common;
        apple.maxStack = 20;
        apple.sellPrice = 5;
        apple.buyPrice = 15;
        apple.texturePath = "assets/consumables/apple.png";
        apple.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 10));
        registerItem(apple);
    }
    
    {
        ItemData cherry;
        cherry.id = "cherry";
        cherry.name = "樱桃";
        cherry.description = "甜美的樱桃，食用后恢复生命值";
        cherry.type = ItemType::Consumable;
        cherry.rarity = ItemRarity::Uncommon;
        cherry.maxStack = 20;
        cherry.sellPrice = 8;
        cherry.buyPrice = 25;
        cherry.texturePath = "assets/consumables/cherry.png";
        cherry.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 20));
        registerItem(cherry);
    }
    
    {
        ItemData healthPotion;
        healthPotion.id = "health_potion";
        healthPotion.name = "生命药水";
        healthPotion.description = "红色的药水，能够快速恢复大量生命值";
        healthPotion.type = ItemType::Consumable;
        healthPotion.rarity = ItemRarity::Rare;
        healthPotion.maxStack = 10;
        healthPotion.sellPrice = 25;
        healthPotion.buyPrice = 100;
        healthPotion.texturePath = "assets/consumables/health_potion.png";
        healthPotion.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 50));
        registerItem(healthPotion);
    }
    
    // 面包
    {
        ItemData bread;
        bread.id = "bread";
        bread.name = "面包";
        bread.description = "谷物做的面包，可以填饱肚子";
        bread.type = ItemType::Consumable;
        bread.rarity = ItemRarity::Common;
        bread.maxStack = 99;
        bread.sellPrice = 60;
        bread.buyPrice = 150;
        bread.texturePath = "assets/consumables/bread.png";
        bread.effects.push_back(ConsumableEffect(EffectType::RestoreStamina, 30));  // 饥饿度增加30点
        registerItem(bread);
    }
    
    // ========================================
    // 装备类物品 (Equipment) - 武器
    // 存放目录: assets/equipment/
    // ========================================
    
    // 斧头 - 无视树木类防御
    {
        ItemData axe;
        axe.id = "axe";
        axe.name = "斧头";
        axe.description = "用于砍伐的工具，可以无视树木类的防御值";
        axe.type = ItemType::Equipment;
        axe.rarity = ItemRarity::Common;  // 白色
        axe.maxStack = 1;
        axe.sellPrice = 50;
        axe.buyPrice = 150;
        axe.texturePath = "assets/weapon/axe.png";
        registerItem(axe);
    }
    
    // 小刀 - 有资质系统
    {
        ItemData knife;
        knife.id = "knife";
        knife.name = "小刀";
        knife.description = "锋利的小刀，可以通过锻造获得不同资质";
        knife.type = ItemType::Equipment;
        knife.rarity = ItemRarity::Common;  // 基础白色，实际资质在装备系统中处理
        knife.maxStack = 1;
        knife.sellPrice = 80;
        knife.buyPrice = 250;
        knife.texturePath = "assets/weapon/knife.png";
        registerItem(knife);
    }
    
    // 长矛 - 有资质系统，无视防御值2
    {
        ItemData spear;
        spear.id = "spear";
        spear.name = "长矛";
        spear.description = "长杆武器，攻击距离较远，可以无视部分防御值";
        spear.type = ItemType::Equipment;
        spear.rarity = ItemRarity::Common;  // 基础白色
        spear.maxStack = 1;
        spear.sellPrice = 120;
        spear.buyPrice = 400;
        spear.texturePath = "assets/weapon/spear.png";
        registerItem(spear);
    }
    
    // ========================================
    // 装备类物品 (Equipment) - 防具
    // ========================================
    
    // 头盔
    {
        ItemData leatherCap;
        leatherCap.id = "leather_cap";
        leatherCap.name = "皮帽";
        leatherCap.description = "简单的皮制帽子";
        leatherCap.type = ItemType::Equipment;
        leatherCap.rarity = ItemRarity::Common;
        leatherCap.maxStack = 1;
        leatherCap.sellPrice = 8;
        leatherCap.buyPrice = 30;
        leatherCap.texturePath = "assets/equipment/leather_cap.png";
        registerItem(leatherCap);
    }
    
    {
        ItemData ironHelmet;
        ironHelmet.id = "iron_helmet";
        ironHelmet.name = "铁盔";
        ironHelmet.description = "坚固的铁制头盔";
        ironHelmet.type = ItemType::Equipment;
        ironHelmet.rarity = ItemRarity::Uncommon;
        ironHelmet.maxStack = 1;
        ironHelmet.sellPrice = 35;
        ironHelmet.buyPrice = 150;
        ironHelmet.texturePath = "assets/equipment/iron_helmet.png";
        registerItem(ironHelmet);
    }
    
    // 铠甲
    {
        ItemData leatherArmor;
        leatherArmor.id = "leather_armor";
        leatherArmor.name = "皮甲";
        leatherArmor.description = "轻便的皮制护甲";
        leatherArmor.type = ItemType::Equipment;
        leatherArmor.rarity = ItemRarity::Common;
        leatherArmor.maxStack = 1;
        leatherArmor.sellPrice = 15;
        leatherArmor.buyPrice = 60;
        leatherArmor.texturePath = "assets/equipment/leather_armor.png";
        registerItem(leatherArmor);
    }
    
    {
        ItemData ironArmor;
        ironArmor.id = "iron_armor";
        ironArmor.name = "铁甲";
        ironArmor.description = "厚重的铁制铠甲";
        ironArmor.type = ItemType::Equipment;
        ironArmor.rarity = ItemRarity::Uncommon;
        ironArmor.maxStack = 1;
        ironArmor.sellPrice = 60;
        ironArmor.buyPrice = 250;
        ironArmor.texturePath = "assets/equipment/iron_armor.png";
        registerItem(ironArmor);
    }
    
    // 裤子
    {
        ItemData leatherPants;
        leatherPants.id = "leather_pants";
        leatherPants.name = "皮裤";
        leatherPants.description = "简单的皮制裤子";
        leatherPants.type = ItemType::Equipment;
        leatherPants.rarity = ItemRarity::Common;
        leatherPants.maxStack = 1;
        leatherPants.sellPrice = 10;
        leatherPants.buyPrice = 40;
        leatherPants.texturePath = "assets/equipment/leather_pants.png";
        registerItem(leatherPants);
    }
    
    // 手套
    {
        ItemData leatherGloves;
        leatherGloves.id = "leather_gloves";
        leatherGloves.name = "皮手套";
        leatherGloves.description = "简单的皮制手套";
        leatherGloves.type = ItemType::Equipment;
        leatherGloves.rarity = ItemRarity::Common;
        leatherGloves.maxStack = 1;
        leatherGloves.sellPrice = 6;
        leatherGloves.buyPrice = 25;
        leatherGloves.texturePath = "assets/equipment/leather_gloves.png";
        registerItem(leatherGloves);
    }
    
    {
        ItemData warriorGloves;
        warriorGloves.id = "warrior_gloves";
        warriorGloves.name = "战士手套";
        warriorGloves.description = "战士专用的强化手套";
        warriorGloves.type = ItemType::Equipment;
        warriorGloves.rarity = ItemRarity::Uncommon;
        warriorGloves.maxStack = 1;
        warriorGloves.sellPrice = 25;
        warriorGloves.buyPrice = 100;
        warriorGloves.texturePath = "assets/equipment/warrior_gloves.png";
        registerItem(warriorGloves);
    }
    
    // 鞋子
    {
        ItemData leatherBoots;
        leatherBoots.id = "leather_boots";
        leatherBoots.name = "皮靴";
        leatherBoots.description = "简单的皮制靴子";
        leatherBoots.type = ItemType::Equipment;
        leatherBoots.rarity = ItemRarity::Common;
        leatherBoots.maxStack = 1;
        leatherBoots.sellPrice = 8;
        leatherBoots.buyPrice = 35;
        leatherBoots.texturePath = "assets/equipment/leather_boots.png";
        registerItem(leatherBoots);
    }
    
    // 披风
    {
        ItemData simpleCape;
        simpleCape.id = "simple_cape";
        simpleCape.name = "简易披风";
        simpleCape.description = "一件普通的披风";
        simpleCape.type = ItemType::Equipment;
        simpleCape.rarity = ItemRarity::Common;
        simpleCape.maxStack = 1;
        simpleCape.sellPrice = 10;
        simpleCape.buyPrice = 45;
        simpleCape.texturePath = "assets/equipment/simple_cape.png";
        registerItem(simpleCape);
    }
    
    // 盾牌
    {
        ItemData woodenShield;
        woodenShield.id = "wooden_shield";
        woodenShield.name = "木盾";
        woodenShield.description = "简单的木制盾牌";
        woodenShield.type = ItemType::Equipment;
        woodenShield.rarity = ItemRarity::Common;
        woodenShield.maxStack = 1;
        woodenShield.sellPrice = 12;
        woodenShield.buyPrice = 50;
        woodenShield.texturePath = "assets/equipment/wooden_shield.png";
        registerItem(woodenShield);
    }
    
    {
        ItemData ironShield;
        ironShield.id = "iron_shield";
        ironShield.name = "铁盾";
        ironShield.description = "坚固的铁制盾牌";
        ironShield.type = ItemType::Equipment;
        ironShield.rarity = ItemRarity::Uncommon;
        ironShield.maxStack = 1;
        ironShield.sellPrice = 40;
        ironShield.buyPrice = 170;
        ironShield.texturePath = "assets/equipment/iron_shield.png";
        registerItem(ironShield);
    }
    
    // ========================================
    // 宠物相关物品 (Pet Items)
    // 存放目录: assets/pet/
    // ========================================
    
    // 兔子精元
    {
        ItemData rabbitEssence;
        rabbitEssence.id = "rabbit_essence";
        rabbitEssence.name = "兔子精元";
        rabbitEssence.description = "蕴含兔子灵魂的神秘精元，可用于孵化宠物兔。掉落概率：2%";
        rabbitEssence.type = ItemType::Material;
        rabbitEssence.rarity = ItemRarity::Common;  // 普通
        rabbitEssence.maxStack = 99;
        rabbitEssence.sellPrice = 500;  // 更新出售价格
        rabbitEssence.buyPrice = 2000;
        rabbitEssence.texturePath = "assets/pet/rabbit_essence.png";
        registerItem(rabbitEssence);
    }
    
    // 臭猪精元
    {
        ItemData stinkyPorkEssence;
        stinkyPorkEssence.id = "stinky_pork_essence";
        stinkyPorkEssence.name = "臭猪精元";
        stinkyPorkEssence.description = "蕴含臭猪灵魂的神秘精元，可用于孵化宠物臭猪";
        stinkyPorkEssence.type = ItemType::Material;
        stinkyPorkEssence.rarity = ItemRarity::Common;  // 普通
        stinkyPorkEssence.maxStack = 99;
        stinkyPorkEssence.sellPrice = 800;
        stinkyPorkEssence.buyPrice = 3200;
        stinkyPorkEssence.texturePath = "assets/materials/stinky_pork_essence.png";
        registerItem(stinkyPorkEssence);
    }
    
    // 孵化强化剂 - 通用（已废弃，保留兼容）
    {
        ItemData hatchEnhancer;
        hatchEnhancer.id = "hatch_enhancer";
        hatchEnhancer.name = "通用强化剂";
        hatchEnhancer.description = "通用的孵化强化剂。注意：每种精元有对应的特殊强化剂材料！";
        hatchEnhancer.type = ItemType::Consumable;
        hatchEnhancer.rarity = ItemRarity::Uncommon;
        hatchEnhancer.maxStack = 99;
        hatchEnhancer.sellPrice = 20;
        hatchEnhancer.buyPrice = 80;
        hatchEnhancer.texturePath = "assets/pet/hatch_enhancer.png";
        registerItem(hatchEnhancer);
    }
    
    // 注：兔毛(rabbit_fur)已在上面定义，它同时作为兔子精元的专用强化剂
    // 每种精元对应的强化剂材料：
    //   - 兔子精元 -> 兔毛 (rabbit_fur)
    //   - 史莱姆精元 -> 粘液 (slime_goo) [未来添加]
    //   - 小鸡精元 -> 羽毛 (feather) [未来添加]
    
    // 宠物洗涤剂
    {
        ItemData petCleanser;
        petCleanser.id = "pet_cleanser";
        petCleanser.name = "宠物洗涤剂";
        petCleanser.description = "可重置宠物资质和属性。将宠物变回1级，重新随机资质（受幸运值影响）";
        petCleanser.type = ItemType::Consumable;
        petCleanser.rarity = ItemRarity::Rare;
        petCleanser.maxStack = 20;
        petCleanser.sellPrice = 100;
        petCleanser.buyPrice = 500;
        petCleanser.texturePath = "assets/pet/pet_cleanser.png";
        registerItem(petCleanser);
    }
    
    // 宠物经验药水
    {
        ItemData petExpPotion;
        petExpPotion.id = "pet_exp_potion";
        petExpPotion.name = "宠物经验药水";
        petExpPotion.description = "给宠物服用可获得500点经验值";
        petExpPotion.type = ItemType::Consumable;
        petExpPotion.rarity = ItemRarity::Uncommon;
        petExpPotion.maxStack = 20;
        petExpPotion.sellPrice = 30;
        petExpPotion.buyPrice = 120;
        petExpPotion.texturePath = "assets/pet/pet_exp_potion.png";
        petExpPotion.effects.push_back(ConsumableEffect(EffectType::BuffExp, 500));
        registerItem(petExpPotion);
    }
    
    // 宠物生命药水
    {
        ItemData petHealthPotion;
        petHealthPotion.id = "pet_health_potion";
        petHealthPotion.name = "宠物生命药水";
        petHealthPotion.description = "给宠物服用可恢复50点生命值";
        petHealthPotion.type = ItemType::Consumable;
        petHealthPotion.rarity = ItemRarity::Common;
        petHealthPotion.maxStack = 20;
        petHealthPotion.sellPrice = 15;
        petHealthPotion.buyPrice = 60;
        petHealthPotion.texturePath = "assets/pet/pet_health_potion.png";
        petHealthPotion.effects.push_back(ConsumableEffect(EffectType::RestoreHealth, 50));
        registerItem(petHealthPotion);
    }
    
    // ========================================
    // 武器强化相关物品
    // 存放目录: assets/materials/
    // ========================================
    
    // 强化石
    {
        ItemData enhanceStone;
        enhanceStone.id = "enhance_stone";
        enhanceStone.name = "强化石";
        enhanceStone.description = "用于强化武器的神秘石头。强化消耗公式：10 + (当前等级² × 2)";
        enhanceStone.type = ItemType::Material;
        enhanceStone.rarity = ItemRarity::Uncommon;  // 良好
        enhanceStone.maxStack = 999;
        enhanceStone.sellPrice = 50;
        enhanceStone.buyPrice = 150;
        enhanceStone.texturePath = "assets/materials/enhance_stone.png";
        registerItem(enhanceStone);
    }
    
    // 武器魂（普通）
    {
        ItemData weaponSoul;
        weaponSoul.id = "weapon_soul";
        weaponSoul.name = "普通武器魂";
        weaponSoul.description = "蕴含武器灵魂的神秘物质。锻造时添加可提升高资质概率（最多30个）";
        weaponSoul.type = ItemType::Material;
        weaponSoul.rarity = ItemRarity::Uncommon;  // 良好
        weaponSoul.maxStack = 99;
        weaponSoul.sellPrice = 100;
        weaponSoul.buyPrice = 300;
        weaponSoul.texturePath = "assets/materials/weapon_soul.png";
        registerItem(weaponSoul);
    }
    
    // 高级武器魂
    {
        ItemData advancedWeaponSoul;
        advancedWeaponSoul.id = "advanced_weapon_soul";
        advancedWeaponSoul.name = "高级武器魂";
        advancedWeaponSoul.description = "更强大的武器魂，相当于3个普通武器魂的效果";
        advancedWeaponSoul.type = ItemType::Material;
        advancedWeaponSoul.rarity = ItemRarity::Rare;  // 优秀
        advancedWeaponSoul.maxStack = 99;
        advancedWeaponSoul.sellPrice = 350;
        advancedWeaponSoul.buyPrice = 1000;
        advancedWeaponSoul.texturePath = "assets/materials/advanced_weapon_soul.png";
        registerItem(advancedWeaponSoul);
    }
    
    initialized = true;
    std::cout << "[ItemDatabase] Registered " << items.size() << " items" << std::endl;
}

bool ItemDatabase::loadTextures(const std::string& basePath) {
    std::cout << "[ItemDatabase] Loading item textures from: " << basePath << std::endl;
    
    int loaded = 0;
    int failed = 0;
    
    for (auto& pair : items) {
        const std::string& itemId = pair.first;
        const ItemData& data = pair.second;
        
        if (data.texturePath.empty()) continue;
        
        sf::Texture texture;
        
        // 尝试多种路径
        std::vector<std::string> paths = {
            data.texturePath,
            basePath + "/" + data.texturePath,
            "../../" + data.texturePath
        };
        
        bool success = false;
        for (const auto& path : paths) {
            if (texture.loadFromFile(path)) {
                textures[itemId] = texture;
                loaded++;
                success = true;
                break;
            }
        }
        
        if (!success) {
            // 创建占位贴图
            sf::Image placeholder;
            placeholder.create(32, 32, sf::Color(100, 100, 100, 200));
            texture.loadFromImage(placeholder);
            textures[itemId] = texture;
            failed++;
            std::cout << "[ItemDatabase] Missing texture for: " << itemId << std::endl;
        }
    }
    
    std::cout << "[ItemDatabase] Loaded " << loaded << " textures, " 
              << failed << " placeholders" << std::endl;
    
    return loaded > 0;
}

const ItemData* ItemDatabase::getItemData(const std::string& itemId) const {
    auto it = items.find(itemId);
    if (it != items.end()) {
        return &it->second;
    }
    return nullptr;
}

const sf::Texture* ItemDatabase::getTexture(const std::string& itemId) const {
    auto it = textures.find(itemId);
    if (it != textures.end()) {
        return &it->second;
    }
    return nullptr;
}

void ItemDatabase::registerItem(const ItemData& data) {
    items[data.id] = data;
    std::cout << "[ItemDatabase] Registered: " << data.id << " (" << data.name << ")" << std::endl;
}

sf::Color ItemDatabase::getRarityColor(ItemRarity rarity) {
    switch (rarity) {
        case ItemRarity::Common:    return sf::Color(255, 255, 255);     // 白色
        case ItemRarity::Uncommon:  return sf::Color(30, 255, 30);       // 绿色
        case ItemRarity::Rare:      return sf::Color(30, 144, 255);      // 蓝色
        case ItemRarity::Epic:      return sf::Color(163, 53, 238);      // 紫色
        case ItemRarity::Legendary: return sf::Color(255, 165, 0);       // 橙色
        default:                    return sf::Color::White;
    }
}

std::string ItemDatabase::getTypeName(ItemType type) {
    switch (type) {
        case ItemType::Material:    return "材料";
        case ItemType::Consumable:  return "消耗品";
        case ItemType::Equipment:   return "装备";
        case ItemType::Quest:       return "任务物品";
        case ItemType::Misc:        return "杂物";
        default:                    return "未知";
    }
}

// ============================================================================
// 辅助函数实现
// ============================================================================

ItemStack createItemStack(const std::string& itemId, int count) {
    return ItemStack(itemId, count);
}

bool canStackItems(const ItemStack& a, const ItemStack& b) {
    if (a.isEmpty() || b.isEmpty()) return false;
    return a.itemId == b.itemId;
}

int mergeItemStacks(ItemStack& dest, ItemStack& src, int maxStack) {
    if (dest.isEmpty()) {
        dest = src;
        src.clear();
        return 0;
    }
    
    if (dest.itemId != src.itemId) {
        return src.count;
    }
    
    int canAdd = maxStack - dest.count;
    int toAdd = std::min(canAdd, src.count);
    
    dest.count += toAdd;
    src.count -= toAdd;
    
    if (src.count <= 0) {
        src.clear();
    }
    
    return src.count;
}
