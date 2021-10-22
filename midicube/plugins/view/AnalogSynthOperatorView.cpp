/*
 * AnalogSynthOperatorView.cpp
 *
 *  Created on: Feb 23, 2021
 *      Author: jojo
 */

#include "../view/AnalogSynthOperatorView.h"

#include "../view/AnalogSynthOscilatorView.h"
#include "../view/AnalogSynthView.h"
#include "../../resources.h"


AnalogSynthOperatorView::AnalogSynthOperatorView(AdvancedSynth &s, size_t part) : synth(s), binder{s.get_lock(), [&s, part]() {
			return new AnalogSynthOperatorView(s, part);
		}, main_font} {
	this->part = part;
}


AnalogSynthOperatorView::~AnalogSynthOperatorView() {

}

static void create_filter_view(int& tmp_x, int& tmp_y, FilterEntity& filter, std::vector<Control*>& controls, std::vector<Control*>& show_amount, std::vector<Control*>& show_source, SpinLock& lock) {
	//Filter
	{
		CheckBox* active = new CheckBox(false, "Filter", main_font, 16, tmp_x, tmp_y + 15, 40, 40);
		active->property.bind(filter.on, lock);
		controls.push_back(active);
	}
	tmp_x += 100;
	//Filter Type
	{
		std::vector<std::string> filter_types = {"LP 12", "LP 24", "HP 12", "HP 24", "BP 12", "BP 24"};

		ComboBox* filter_type = new ComboBox(1, filter_types, main_font, 16, 0, tmp_x, tmp_y + 15, 150, 40);
		filter_type->property.bind_cast(filter.type, lock);
		controls.push_back(filter_type);
	}
	tmp_x += 160;
	//KB Track
	{
		Label* title = new Label("KB Track", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<double>* value = new DragBox<double>(0, 0, 1, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->property.bind(filter.kb_track, lock);
		controls.push_back(value);
	}
	tmp_x += 90;
	//KB Track Note
	{
		Label* title = new Label("KB Track Note", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<int>* value = new DragBox<int>(0, 0, 127, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->property.bind(filter.kb_track_note, lock);
		controls.push_back(value);
	}
	tmp_x += 90;
	tmp_y += 65;

	tmp_x = 500;
	//Drive
	{
		CheckBox* active = new CheckBox(false, "Drive", main_font, 16, tmp_x, tmp_y + 15, 40, 40);
		active->property.bind(filter.drive, lock);
		controls.push_back(active);
	}
	tmp_x += 90;
	//Drive Amount
	{
		Label* title = new Label("Drive Amt", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<double>* value = new DragBox<double>(0, 0, 1, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->property.bind(filter.drive_amount, lock);
		controls.push_back(value);
	}
	tmp_x += 90;

	tmp_y += 65;

	tmp_x = 500;
	//Filter Cutoff
	DragBox<double>* cutoff = property_mod_controls(&controls, tmp_x, tmp_y, filter.cutoff, lock, "Cutoff", &show_amount, &show_source).at(0);
	cutoff->to_string = [](double val) {
		return std::to_string((int) scale_cutoff(val));
	};
	cutoff->drag_step = 4;
	tmp_y += 65;
	//Filter Resonance
	property_mod_controls(&controls, tmp_x, tmp_y, filter.resonance, lock, "Resonance", &show_amount, &show_source);
	tmp_y += 65;
}

Scene AnalogSynthOperatorView::create(ViewHost &frame) {
	std::vector<Control*> controls;
	std::vector<Control*> show_amount;
	std::vector<Control*> show_source;

	SpinLock& lock = synth.get_lock();
	OperatorEntity& op = synth.preset.operators.at(this->part);

	//Background
	Pane* bg = new Pane(sf::Color(80, 80, 80), 0, 0, frame.get_width(), frame.get_height());
	controls.push_back(bg);

	Pane* pane = new Pane(sf::Color(120, 120, 120), 5, 5, frame.get_width() - 10, frame.get_height() - 50);
	controls.push_back(pane);

	int tmp_x = 10;
	int tmp_y = 10;

	//Col 1
	{
		CheckBox* audible = new CheckBox(false, "Audible", main_font, 16, tmp_x, tmp_y + 15, 40, 40);
		audible->property.bind(op.audible, lock);
		controls.push_back(audible);
	}

	{
		Label* title = new Label("Osc. Count", main_font, 12, tmp_x + 160, tmp_y);
		controls.push_back(title);

		DragBox<int>* value = new DragBox<int>(0, 0, 8, main_font, 16, tmp_x + 160, tmp_y + 15, 80, 40);
		value->property.bind(op.oscilator_count, lock);
		controls.push_back(value);
	}

	tmp_y += 65;

	//ADSR
	adsr_controls(&controls, tmp_x, tmp_y, op.env, lock);
	tmp_y += 225;
	//Volume
	property_mod_controls(&controls, tmp_x, tmp_y, op.volume, lock, "Volume", &show_amount, &show_source);
	tmp_y += 65;
	//Panning
	property_mod_controls(&controls, tmp_x, tmp_y, op.panning, lock, "Panning", &show_amount, &show_source);
	tmp_y += 65;

	//Octave Amp
	{
		Label* title = new Label("Amp KB Upper", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<double>* value = new DragBox<double>(0, -1, 1, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->property.bind(op.amp_kb_track_upper, lock);
		controls.push_back(value);
	}
	tmp_x += 90;
	//Octave Amp
	{
		Label* title = new Label("Amp KB Lower", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<double>* value = new DragBox<double>(0, -1, 1, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->property.bind(op.amp_kb_track_lower, lock);
		controls.push_back(value);
	}
	tmp_x += 90;
	//Amp KB Track Center
	{
		Label* title = new Label("Amp KB Track Note", main_font, 12, tmp_x, tmp_y);
		controls.push_back(title);

		DragBox<int>* value = new DragBox<int>(0, 0, 127, main_font, 16, tmp_x, tmp_y + 15, 80, 40);
		value->property.bind(op.amp_kb_track_note, lock);
		controls.push_back(value);
	}
	tmp_x += 90;

	//Filter Parallel
	{
		CheckBox* parallel = new CheckBox(false, "Filter Parallel", main_font, 16, tmp_x, tmp_y + 15, 40, 40);
		parallel->property.bind(op.filter_parallel, lock);
		controls.push_back(parallel);
	}

	//Col 2
	tmp_x = 500;
	tmp_y = 10;

	//Filter Section
	create_filter_view(tmp_x, tmp_y, op.first_filter, controls, show_amount, show_source, lock);
	tmp_y += 10;
	create_filter_view(tmp_x, tmp_y, op.second_filter, controls, show_amount, show_source, lock);

	//Edit Sources
	Button* edit = new Button("Edit Sources", main_font, 18, frame.get_width() - 70 - 120, frame.get_height() - 40, 120, 40);
	edit->rect.setFillColor(sf::Color::Yellow);
	edit->set_on_click([&frame, show_amount, show_source, this]() {
		edit_source = !edit_source;
		for (Control* c : show_amount) {
			c->set_visible(!edit_source);
		}
		for (Control* c : show_source) {
			c->set_visible(edit_source);
		}
	});
	controls.push_back(edit);

	controls.push_back(binder.create_button(frame.get_width() - 70 - 120 - 100, frame.get_height() - 40, &frame));
	//Back Button
	Button* back = new Button("Back", main_font, 18, frame.get_width() - 70, frame.get_height() - 40, 70, 40);
	back->rect.setFillColor(sf::Color::Yellow);
	back->set_on_click([&frame, this]() {
		frame.change_view(new AnalogSynthView(synth));
	});
	controls.push_back(back);

	for (Control* c : show_amount) {
		c->set_visible(!edit_source);
	}
	for (Control* c : show_source) {
		c->set_visible(edit_source);
	}

	return {controls};
}

bool AnalogSynthOperatorView::on_action(Control *control) {
	return binder.on_action(control);
}
