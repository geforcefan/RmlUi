#pragma once

#include "../../../Include/RmlUi/Core/Types.h"

namespace Rml {

class Element;

/*
    Memoizes content-based size measurements for the duration of a single layout pass.

    Sizing a flex container measures the content-based size of each auto-sized flex item by formatting the item's
    subtree, in addition to the item's final formatting. In nested flexboxes each level repeats the measurements of its
    descendants, so the number of times a subtree is formatted grows exponentially with the number of flex ancestors.
    The measured values only depend on the element's subtree and the given size constraints, both of which are fixed
    during a layout pass, which makes them safe to reuse until the pass is completed.

    The cache registers itself as active during its lifetime, and is intended to be instantiated at the root of each
    layout pass.
*/
class LayoutMeasureCache final {
public:
	LayoutMeasureCache();
	~LayoutMeasureCache();

	/// Returns the currently active cache, or nullptr if no layout pass is in progress.
	static LayoutMeasureCache* GetActiveCache();

	/// Retrieves the shrink-to-fit width of an element, as previously measured under the same containing block.
	bool FindShrinkToFitWidth(Element* element, Vector2f containing_block, float& shrink_to_fit_width) const;
	void StoreShrinkToFitWidth(Element* element, Vector2f containing_block, float shrink_to_fit_width);

	/// Retrieves the max-content size of a flex container, as previously measured.
	bool FindMaxContentSize(Element* element, Vector2f& max_content_size) const;
	void StoreMaxContentSize(Element* element, Vector2f max_content_size);

	/// Retrieves the content height of an element, as previously formatted under the same definite content width.
	bool FindFormattedContentHeight(Element* element, float content_width, float& formatted_content_height) const;
	void StoreFormattedContentHeight(Element* element, float content_width, float formatted_content_height);

private:
	struct ShrinkToFitWidthEntry {
		Vector2f containing_block;
		float shrink_to_fit_width;
	};
	struct FormattedContentHeightEntry {
		float content_width;
		float formatted_content_height;
	};

	UnorderedMap<Element*, Vector<ShrinkToFitWidthEntry>> shrink_to_fit_widths;
	UnorderedMap<Element*, Vector2f> max_content_sizes;
	UnorderedMap<Element*, Vector<FormattedContentHeightEntry>> formatted_content_heights;

	LayoutMeasureCache* previous_active_cache;
};

} // namespace Rml
