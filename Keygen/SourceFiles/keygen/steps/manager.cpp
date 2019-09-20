// This file is part of TON Key Generator,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL
//
#include "keygen/steps/manager.h"

#include "keygen/steps/intro.h"
#include "keygen/steps/random_seed.h"
#include "keygen/steps/created.h"
#include "keygen/steps/view.h"
#include "keygen/steps/check.h"
#include "keygen/steps/done.h"
#include "keygen/phrases.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/text/text_utilities.h"
#include "ui/rp_widget.h"
#include "ui/message_box.h"
#include "styles/style_keygen.h"

namespace Keygen::Steps {
namespace {

constexpr auto kSeedLengthMin = 50;
constexpr auto kSeedLengthMax = 200;

} // namespace

Manager::Manager()
: _content(std::make_unique<Ui::RpWidget>())
, _nextButton(
	std::in_place,
	_content.get(),
	object_ptr<Ui::RoundButton>(
		_content.get(),
		rpl::single(QString()),
		st::nextButton))
, _layerManager(_content.get()) {
	_nextButton->entity()->clicks(
	) | rpl::start_with_next([=] {
		next();
	}, _nextButton->lifetime());
}

Manager::~Manager() = default;

not_null<Ui::RpWidget*> Manager::content() const {
	return _content.get();
}

void Manager::next() {
	if (_next) {
		_next();
	}
}

void Manager::back() {
	if (_back) {
		_back();
	}
}

void Manager::showIntro() {
	showStep(std::make_unique<Intro>(), [=] {
		showRandomSeed();
	});
}

void Manager::showRandomSeed() {
	using namespace rpl::mappers;

	auto seed = std::make_unique<RandomSeed>();

	const auto raw = seed.get();
	raw->length(
	) | rpl::filter(
		_1 >= kSeedLengthMin
	) | rpl::take(
		1
	) | rpl::start_with_next([=] {
		raw->showLimit(kSeedLengthMax);
	}, raw->widget()->lifetime());

	showStep(std::move(seed), [=] {
		const auto text = raw->accumulated();
		if (text.size() >= kSeedLengthMin) {
			_generateRequests.fire(text.toUtf8());
		}
	}, [=] {
		showIntro();
	});
}

void Manager::showCreated(std::vector<QString> &&words) {
	auto next = [=, list = std::move(words)]() mutable {
		showWords(std::move(list));
	};
	showStep(std::make_unique<Created>(), std::move(next), [=] {
		showRandomSeed();
	});
}

void Manager::showWords(std::vector<QString> &&words) {
	_words = words;
	showStep(std::make_unique<View>(std::move(words)), [=] {
		showCheck();
	});
}

void Manager::showCheck() {
	//auto check = std::make_unique<Check>(); // #TODO don't pass words
	auto words = _words;
	words.pop_back();
	auto check = std::make_unique<Check>(words);

	const auto raw = check.get();
	auto back = _words.empty()
		? Fn<void()>()
		: [=] { showWords(base::duplicate(_words)); };
	showStep(std::move(check), [=] {
		_checkRequests.fire(raw->words());
	}, back);
}

void Manager::showCheckDone(const QString &publicKey) {
	showBox(Box([=](not_null<Ui::GenericBox*> box) {
		Ui::InitMessageBox(
			box,
			tr::lng_check_good_title(),
			tr::lng_check_good_text(Ui::Text::RichLangValue));
		box->addButton(
			tr::lng_check_good_next(),
			[=] { showDone(publicKey); });
	}));
}

void Manager::showCheckFail() {
	showBox(Box([=](not_null<Ui::GenericBox*> box) {
		Ui::InitMessageBox(
			box,
			tr::lng_check_bad_title(),
			tr::lng_check_bad_text(Ui::Text::RichLangValue));
		box->addButton(
			tr::lng_check_bad_try_again(),
			[=] { box->closeBox(); });
		box->addButton(tr::lng_check_bad_view_words(), [=] { back(); });
	}));
}

void Manager::showDone(const QString &publicKey) {
	showStep(std::make_unique<Done>(publicKey));
}

void Manager::showBox(
		object_ptr<Ui::BoxContent> box,
		Ui::LayerOptions options,
		anim::type animated) {
	_layerManager.showBox(std::move(box), options, animated);
}

void Manager::showStep(
		std::unique_ptr<Step> step,
		FnMut<void()> next,
		FnMut<void()> back) {
	_layerManager.hideAll();

	_step = std::move(step);
	_next = std::move(next);
	_back = std::move(back);

	const auto inner = _step->widget();
	inner->setParent(_content.get());
	_content->sizeValue(
	) | rpl::start_with_next([=](QSize size) {
		inner->setGeometry({ QPoint(), size });
	}, inner->lifetime());
	inner->show();

	_nextButton->entity()->setText(_step->nextButtonState(
	) | rpl::filter([](const NextButtonState &state) {
		return !state.text.isEmpty();
	}) | rpl::map([](const NextButtonState &state) {
		return state.text;
	}));
	_nextButton->toggleOn(_step->nextButtonState(
	) | rpl::map([](const NextButtonState &state) {
		return !state.text.isEmpty();
	}));
	_nextButton->raise();
	_layerManager.raise();

	rpl::combine(
		_step->nextButtonState(),
		_content->widthValue()
	) | rpl::start_with_next([=](NextButtonState state, int width) {
		_nextButton->move(
			(width - _nextButton->width()) / 2,
			state.top);
		_lastNextState = state;
	}, inner->lifetime());

	_step->nextClicks(
	) | rpl::start_with_next([=] {
		this->next();
	}, inner->lifetime());

	_step->setFocus();
}

rpl::producer<QByteArray> Manager::generateRequests() const {
	return _generateRequests.events();
}

rpl::producer<std::vector<QString>> Manager::checkRequests() const {
	return _checkRequests.events();
}

} // namespace Keygen::Steps