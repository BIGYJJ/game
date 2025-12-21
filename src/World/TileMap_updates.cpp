// ============================================================================
// TileMap.cpp 更新说明
// 
// 在 loadTsxFile 方法的属性解析部分（大约第740行之后）添加以下代码
// ============================================================================

// 找到这段代码（大约在第774-779行）:
//
// } else if (propName.find("drop") != std::string::npos && 
//            propName.find("Probability") != std::string::npos) {
//     // dropX_Probability
//     float prob = std::stof(propValue);
//     prop.dropProbabilities.push_back(prob);
// }
//
// 在这段代码之后，propPos = propEnd; 之前，添加以下新的属性解析:

/*
                // === 新增属性解析 ===
                
                // 基类类型 (build, plant, etc.)
                } else if (propName == "base") {
                    prop.base = propValue;
                    
                // 植物相关属性
                } else if (propName == "allow_pick" || propName == "allow_pickup") {
                    prop.allowPickup = (propValue == "true" || propValue == "1");
                    
                } else if (propName == "pickup_object") {
                    // 移除可能的引号
                    std::string pickupStr = propValue;
                    if (pickupStr.size() >= 2 && pickupStr.front() == '"' && pickupStr.back() == '"') {
                        pickupStr = pickupStr.substr(1, pickupStr.size() - 2);
                    }
                    prop.pickupObject = pickupStr;
                    
                } else if (propName == "count_min") {
                    prop.countMin = std::stoi(propValue);
                    
                } else if (propName == "count_max") {
                    prop.countMax = std::stoi(propValue);
                    
                } else if (propName == "probability") {
                    prop.probability = std::stof(propValue);
                }
*/

// ============================================================================
// 完整的属性解析代码块示例（包含所有新旧属性）
// ============================================================================

// 解析各种属性
/*
if (propName == "name") {
    prop.name = propValue;
} else if (propName == "type") {
    prop.type = propValue;
} else if (propName == "base") {
    prop.base = propValue;
} else if (propName == "HP") {
    prop.hp = std::stoi(propValue);
} else if (propName == "defense") {
    prop.defense = std::stoi(propValue);
} else if (propName == "drop_max") {
    prop.dropMax = (int)std::stof(propValue);
} else if (propName == "exp_min") {
    prop.expMin = std::stoi(propValue);
} else if (propName == "exp_max") {
    prop.expMax = std::stoi(propValue);
} else if (propName == "gold_min") {
    prop.goldMin = std::stoi(propValue);
} else if (propName == "gold_max") {
    prop.goldMax = std::stoi(propValue);
} else if (propName == "drop_type") {
    // 解析掉落类型列表，格式: "\"wood\",\"seed\",\"stick\""
    std::string dropStr = propValue;
    size_t pos = 0;
    while (pos < dropStr.length()) {
        size_t start = dropStr.find("\"", pos);
        if (start == std::string::npos) break;
        size_t end = dropStr.find("\"", start + 1);
        if (end == std::string::npos) break;
        std::string item = dropStr.substr(start + 1, end - start - 1);
        if (!item.empty()) {
            prop.dropTypes.push_back(item);
        }
        pos = end + 1;
    }
} else if (propName.find("drop") != std::string::npos && 
           propName.find("Probability") != std::string::npos) {
    // dropX_Probability
    float prob = std::stof(propValue);
    prop.dropProbabilities.push_back(prob);
// === 新增植物相关属性 ===
} else if (propName == "allow_pick" || propName == "allow_pickup") {
    prop.allowPickup = (propValue == "true" || propValue == "1");
} else if (propName == "pickup_object") {
    std::string pickupStr = propValue;
    if (pickupStr.size() >= 2 && pickupStr.front() == '"' && pickupStr.back() == '"') {
        pickupStr = pickupStr.substr(1, pickupStr.size() - 2);
    }
    prop.pickupObject = pickupStr;
} else if (propName == "count_min") {
    prop.countMin = std::stoi(propValue);
} else if (propName == "count_max") {
    prop.countMax = std::stoi(propValue);
} else if (propName == "probability") {
    prop.probability = std::stof(propValue);
}
*/

// ============================================================================
// 解析碰撞盒（objectgroup）
// 在属性解析之后，<image>解析之前添加：
// ============================================================================

/*
// 解析碰撞盒 (objectgroup)
size_t objGrpPos = tileXml.find("<objectgroup");
if (objGrpPos != std::string::npos) {
    size_t objPos = tileXml.find("<object", objGrpPos);
    if (objPos != std::string::npos) {
        size_t objEnd = tileXml.find("/>", objPos);
        if (objEnd == std::string::npos) objEnd = tileXml.find(">", objPos);
        
        std::string objTag = tileXml.substr(objPos, objEnd - objPos);
        
        std::string xStr = getXmlAttrStr(objTag, "x");
        std::string yStr = getXmlAttrStr(objTag, "y");
        std::string wStr = getXmlAttrStr(objTag, "width");
        std::string hStr = getXmlAttrStr(objTag, "height");
        
        if (!xStr.empty() && !yStr.empty() && !wStr.empty() && !hStr.empty()) {
            prop.hasCollisionBox = true;
            prop.collisionX = std::stof(xStr);
            prop.collisionY = std::stof(yStr);
            prop.collisionWidth = std::stof(wStr);
            prop.collisionHeight = std::stof(hStr);
            
            std::cout << "       collision box: (" << prop.collisionX << ", " 
                      << prop.collisionY << ", " << prop.collisionWidth << ", " 
                      << prop.collisionHeight << ")" << std::endl;
        }
    }
}
*/

// ============================================================================
// 更新调试输出（可选）
// ============================================================================

/*
// 输出调试信息
if (!prop.name.empty()) {
    std::cout << "     [Tile " << prop.localId << "] name=" << prop.name 
              << ", type=" << prop.type << ", base=" << prop.base
              << ", HP=" << prop.hp << ", defense=" << prop.defense << std::endl;
    
    // 植物属性
    if (prop.allowPickup) {
        std::cout << "       pickup: " << prop.pickupObject 
                  << " (" << prop.countMin << "-" << prop.countMax << ")"
                  << ", prob=" << prop.probability << std::endl;
    }
    
    if (!prop.dropTypes.empty()) {
        std::cout << "       drops: ";
        for (size_t i = 0; i < prop.dropTypes.size(); i++) {
            std::cout << prop.dropTypes[i];
            if (i < prop.dropProbabilities.size()) {
                std::cout << "(" << (int)(prop.dropProbabilities[i] * 100) << "%)";
            }
            if (i < prop.dropTypes.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
}
*/
