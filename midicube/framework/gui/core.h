/*
 * core.h
 *
 *  Created on: Dec 19, 2020
 *      Author: jojo
 */

#ifndef MIDICUBE_GUI_ENGINE_CORE_H_
#define MIDICUBE_GUI_ENGINE_CORE_H_

#include <SFML/Graphics.hpp>
#include <functional>
#include "../util/util.h"
#include "../data/data.h"

#define SELECTABLE virtual bool selectable() const { return true; };



class ViewHost;

class Control {
private:
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	bool visible = true;
	ViewHost* host = nullptr;

public:

	Control(int x = 0, int y = 0, int width = 0, int height = 0);

	virtual void init(ViewHost* host);

	virtual void update_position(int x, int y, int width, int height);

	virtual void draw(sf::RenderWindow& window, bool selected) = 0;

	bool collides (int x, int y);

	virtual void update_properties() {

	}

	virtual bool selectable() const {
		return false;
	}

	virtual void on_mouse_pressed(int x, int y, sf::Mouse::Button button) {

	}

	virtual void on_mouse_drag(int x, int y, int x_motion, int y_motion) {

	}

	virtual void on_mouse_released(int x, int y, sf::Mouse::Button button) {

	}

	virtual void on_mouse_action() {

	}

	virtual ~Control() {

	}

	bool is_visible() const;

	void set_visible(bool visible = true);
};

struct Scene {
	std::vector<Control*> controls;
};

class ViewController {
public:
	ViewController() {

	}

	virtual Scene create(ViewHost& frame) = 0;

	virtual void update_properties() {

	}

	virtual bool on_action(Control* control) {
		return true;
	}

	virtual ~ViewController() {

	}
};

class ViewHost {
private:
	ViewController* view = nullptr;
	std::vector<Control*> controls;

public:
	ViewHost() {

	}

	virtual void change_view(ViewController* view) = 0;

	virtual int get_x_offset() const = 0;

	virtual int get_y_offset() const = 0;

	virtual int get_height() const = 0;

	virtual int get_width() const = 0;

	virtual std::vector<Control*> get_controls() {
		return controls;
	}

	virtual void add_control(Control* control) {
		if (control == nullptr) {
			throw "Can't add nullptr control!";
		}
		else if (std::find(controls.begin(), controls.end(), control) == controls.end()) {
			control->init(this);
			controls.push_back(control);
		}
		else {
			throw "Can't add same control to frame twice!";
		}
	}

	virtual ~ViewHost() {
		delete view;
		for (Control* control : controls) {
			delete control;
		}
	}

protected:
	virtual void switch_view(ViewController* view);

};

class Frame {
private:
	int width;
	int height;
	std::string title;
	ViewController* view;
	std::vector<Control*> controls;

	bool mouse_pressed = false;
	int last_mouse_x = 0;
	int last_mouse_y = 0;
	Control* selected;

	bool redraw = true;
	ViewController* next_view = nullptr;
	bool request_close = false;

	bool render_sleep = true;

public:
	Frame(int width, int height, std::string title, bool render_sleep = true);

	void run(ViewController* v);

	void update_properties();

	void request_redraw() {
		redraw = true;
	}

	void close() {
		request_close = true;
	}

	void change_view(ViewController* view) {
		delete next_view;
		next_view = view;
	}

	virtual ~Frame();

	int get_height() const {
		return height;
	}

	int get_width() const {
		return width;
	}

private:
	void switch_view(ViewController* view);

};

#endif /* MIDICUBE_GUI_ENGINE_CORE_H_ */
