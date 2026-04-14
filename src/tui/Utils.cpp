#include "Utils.h"

namespace tui::utils {

	float Range::Normalized(float value) {
		const float w = maximum - minimum;
		return (value - minimum) / w;
	}

	float Range::Remap(Range other, float value) {
		const float w = maximum - minimum;
		float n = other.Normalized(value);
		return minimum + n * w;
	}

	float Range::Constrain(float value) {
		return std::max(std::min(value, maximum), minimum);
	}

    std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> strings;
    
        std::string::size_type pos = 0;
        std::string::size_type prev = 0;
        while ((pos = str.find(delimiter, prev)) != std::string::npos) {
            strings.push_back(str.substr(prev, pos - prev));
            prev = pos + 1;
        }
    
        // To get the last substring (or only, if delimiter is not found)
        strings.push_back(str.substr(prev));
    
        return strings;
    }
}