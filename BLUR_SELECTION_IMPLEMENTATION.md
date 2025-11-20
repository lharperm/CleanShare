# Selection-Based Blur Implementation

## Overview
The blur slider and selection mask blur have been optimized and properly integrated. The system now supports:
1. **Debounced blur slider** - No lag when dragging the slider
2. **Selection-based blur** - Apply blur only to selected areas using brush strokes
3. **Add/Remove mask modes** - Add to or remove from blur regions using keyboard modifiers

## Architecture

### SessionController (src/core/src/SessionController.cpp)
- **applyFakeBlur(int strength)** - Applies Gaussian blur to entire image
- **applyFakeBlur(int strength, const QImage &mask)** - Applies blur only within the mask region
  - Starts from the **original unblurred image** each time
  - Merges with cumulative mask to preserve previously-blurred areas
  - Updates `m_cumulativeBlurMask` to track all blurred regions
- **removeBlur(const QImage &mask)** - Removes blur from specified regions
  - Subtracts the removal mask from the cumulative blur mask
  - Rebuilds the image with only the non-removed blurred areas

### ImageCanvas (src/presentation/src/ImageCanvas.cpp)
- Users can draw/paint selections with the mouse
- **Modifiers**:
  - No modifier: Replace mode (clear previous selection)
  - Shift: Add mode (add to existing selection)
  - Ctrl: Subtract/Remove mode (remove from blur)
- **Signals**:
  - `selectionChanged(const QImage &mask)` - Emitted when selection changes
  - `selectionModeChanged(bool addMode)` - Emitted with add/subtract mode

### MainWindow (src/presentation/src/MainWindow.cpp)
- **Blur slider debouncing**: 100ms delay prevents lag
  - `onBlurSliderChanged()` - Records pending value, restarts timer
  - `onBlurDebounceTimeout()` - Applies blur after delay
  - `sliderReleased` - Triggers immediate blur when user stops dragging
- **Selection handling**:
  - `onSelectionChanged()` - Applies or removes blur based on mode
  - `onSelectionModeChanged()` - Tracks whether to add or remove

## Workflow

### To apply blur to a selection:
1. Enable "Manual Edit: On" button
2. Draw on the image to create a selection (blue highlight shows selection)
3. Blur is applied at the current slider strength
4. Can adjust slider afterward to change blur amount

### To add to blur region:
1. Hold Shift while drawing to add more areas to blur

### To remove blur from region:
1. Hold Ctrl while drawing to subtract areas (un-blur them)

### Future AI Integration:
- AI detection will create initial mask
- Users can manually add/remove from AI's selection
- Undo/Redo stack supports all operations

## Performance
- Gaussian blur uses separable convolution (fast)
- Blur only recalculates on debounce timeout or slider release
- Mask operations are pixel-by-pixel but efficient
- No lag during slider dragging - UI remains responsive

## Key Implementation Details

### Cumulative Mask Tracking
```
m_cumulativeBlurMask tracks which pixels have been blurred
- Updated when blur is applied (union operation)
- Updated when blur is removed (subtraction operation)
- Allows preservation of previous blur operations
```

### Always Start from Original
```
Each mask-based blur operation:
1. Starts with m_original (unblurred)
2. Generates blurred version at requested radius
3. Blends blurred version into result where mask is white
4. Merges mask with cumulative mask for next operation
```

This prevents blur from compounding and ensures consistent results.
