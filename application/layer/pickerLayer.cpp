#include "core.h"

#include "pickerLayer.h"

#include "view/screen.h"

#include "../picker/picker.h"

#include "input/standardInputHandler.h"

void PickerLayer::onInit() {
	Picker::onInit();
}

void PickerLayer::onUpdate() {
	Screen* screen = static_cast<Screen*>(this->ptr);

	Picker::onUpdate(*screen, handler->mousePosition);
}

void PickerLayer::onEvent(Event& event) {
	Picker::onEvent(event);
}

void PickerLayer::onRender() {
	Screen* screen = static_cast<Screen*>(this->ptr);

	Picker::onRender(*screen);
}

void PickerLayer::onClose() {
	Picker::onClose();
}
