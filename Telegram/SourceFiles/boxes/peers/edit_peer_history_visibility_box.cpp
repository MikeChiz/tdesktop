/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "boxes/peers/edit_peer_history_visibility_box.h"

#include "boxes/peers/edit_peer_permissions_box.h"
#include "boxes/peers/edit_participants_box.h"
#include "data/data_channel.h"
#include "data/data_chat.h"
#include "data/data_peer.h"
#include "lang/lang_keys.h"
#include "styles/style_boxes.h"
#include "styles/style_info.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/padding_wrap.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"

namespace {

std::shared_ptr<Ui::RadioenumGroup<HistoryVisibility>> historyVisibility;
Ui::SlideWrap<Ui::RpWidget> *historyVisibilityWrap = nullptr;

void AddRoundButton(
	not_null<Ui::VerticalLayout*> container,
	HistoryVisibility value,
	LangKey groupTextKey,
	LangKey groupAboutKey) {
	container->add(object_ptr<Ui::FixedHeightWidget>(
		container,
		st::editPeerHistoryVisibilityTopSkip));
	container->add(object_ptr<Ui::Radioenum<HistoryVisibility>>(
		container,
		historyVisibility,
		value,
		lang(groupTextKey),
		st::defaultBoxCheckbox));
	container->add(object_ptr<Ui::PaddingWrap<Ui::FlatLabel>>(
		container,
		object_ptr<Ui::FlatLabel>(
			container,
			Lang::Viewer(groupAboutKey),
			st::editPeerPrivacyLabel),
		st::editPeerPrivacyLabelMargins));
};

void FillContent(
	not_null<Ui::VerticalLayout*> parent,
	not_null<PeerData*> peer,
	std::optional<HistoryVisibility> savedValue = std::nullopt) {

	const auto canEdit = [&] {
		if (const auto chat = peer->asChat()) {
			return chat->canEditPreHistoryHidden();
		} else if (const auto channel = peer->asChannel()) {
			return channel->canEditPreHistoryHidden();
		}
		Unexpected("User in HistoryVisibilityEdit.");
	}();
	if (!canEdit) {
		return;
	}

	const auto channel = peer->asChannel();
	
	const auto result = parent->add(object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
		parent,
		object_ptr<Ui::VerticalLayout>(parent),
		st::editPeerHistoryVisibilityMargins));
	historyVisibilityWrap = result;
	const auto container = result->entity();

	const auto defaultValue = savedValue.value_or(
		(!channel || channel->hiddenPreHistory())
			? HistoryVisibility::Hidden
			: HistoryVisibility::Visible
	);

	historyVisibility = std::make_shared<Ui::RadioenumGroup<HistoryVisibility>>(defaultValue);

	AddRoundButton(
		container,
		HistoryVisibility::Visible,
		lng_manage_history_visibility_shown,
		lng_manage_history_visibility_shown_about);
	AddRoundButton(
		container,
		HistoryVisibility::Hidden,
		lng_manage_history_visibility_hidden,
		(peer->isChat()
			? lng_manage_history_visibility_hidden_legacy
			: lng_manage_history_visibility_hidden_about));
}

} // namespace

EditPeerHistoryVisibilityBox::EditPeerHistoryVisibilityBox(
		QWidget*,
		not_null<PeerData*> peer,
		FnMut<void(HistoryVisibility)> savedCallback,
		std::optional<HistoryVisibility> historyVisibilitySavedValue)
: _peer(peer)
, _savedCallback(std::move(savedCallback))
, _historyVisibilitySavedValue(historyVisibilitySavedValue) {
}

void EditPeerHistoryVisibilityBox::prepare() {
	_peer->updateFull();

	setTitle(langFactory(lng_manage_history_visibility_title));
	addButton(langFactory(lng_settings_save), [=] {
		auto local = std::move(_savedCallback);
		local(historyVisibility->value());
		closeBox();
	});
	addButton(langFactory(lng_cancel), [=] { closeBox(); });

	setupContent();
}

void EditPeerHistoryVisibilityBox::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	FillContent(content, _peer, _historyVisibilitySavedValue);
	setDimensionsToContent(st::boxWidth, content);
}