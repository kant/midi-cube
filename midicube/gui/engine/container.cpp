/*
 * container.cpp
 *
 *  Created on: 10 Oct 2020
 *      Author: jojo
 */

#include "container.h"
#include <cmath>

//VBox
void VBox::update_layout(int parent_width, int parent_height) {
	Node::update_layout(parent_width, parent_height);
	//Calculate values for layout weight
	int weight_sum = 0;
	int height_sum = layout.padding_top;
	for (Node* node : get_children()) {
		//Only if visible
		if (node->is_visible()) {
			NodeLayout& layout = node->get_layout();
			height_sum += layout.margin_top;
			if (layout.height == MATCH_PARENT) {
				weight_sum += layout.y_weight;
			}
			else {
				height_sum += node->calc_size(width - this->layout.padding_left - this->layout.padding_right, height - this->layout.padding_top - this->layout.padding_bottom, true).y;
			}
			height_sum += layout.margin_bottom;
		}
	}
	height_sum += layout.padding_bottom;
	int rest_height = height - height_sum;
	//Layout vertically
	int curr_y = layout.padding_left;
	for (Node* node : get_children()) {
		if (node->is_visible()) {
			NodeLayout& layout = node->get_layout();

			//Position
			curr_y += layout.margin_top;
			float factor = weight_sum == 0 ? 0 : (float) layout.y_weight/weight_sum;
			node->update_layout(width - this->layout.padding_left - this->layout.padding_right, round(rest_height * factor));
			//Alignment
			int x = 0;
			switch (layout.halignment) {
			case HorizontalAlignment::LEFT:
				x = this->layout.padding_left;
				break;
			case HorizontalAlignment::CENTER:
				x = width/2 - node->get_width()/2;
				break;
			case HorizontalAlignment::RIGHT:
				x = width - this->layout.padding_right - width;
				break;
			}
			node->update_position(x, curr_y);
			curr_y += node->get_height();
			curr_y += layout.margin_bottom;
		}

	}
}

void VBox::draw(int parentX, int parentY, NodeEnv env) {
	DrawRectangle(parentX + x, parentY + y, width, height, style.fill_color);
	Parent::draw(parentX, parentY, env);
}

Vector VBox::get_content_size() {
	int curr_y = 0;
	int curr_x = 0;
	for (Node* node : get_children()) {
		if (node->is_visible()) {
			Vector size = node->calc_size(0, 0, true);

			curr_y += node->get_layout().margin_top;
			curr_y += size.y;
			curr_y += node->get_layout().margin_bottom;

			if (curr_x < size.x) {
				curr_x = size.x;
			}
		}
	}
	return {curr_x, curr_y};
}

//HBox
void HBox::update_layout(int parent_width, int parent_height) {
	Node::update_layout(parent_width, parent_height);
	//Calculate values for layout weight
	int weight_sum = 0;
	int width_sum = layout.padding_left;
	for (Node* node : get_children()) {
		if (node->is_visible()) {
			NodeLayout& layout = node->get_layout();
			width_sum += layout.margin_left;
			if (layout.width == MATCH_PARENT) {
				weight_sum += layout.x_weight;
			}
			else {
				width_sum += node->calc_size(width - this->layout.padding_left - this->layout.padding_right, height - this->layout.padding_top - this->layout.padding_bottom, true).x;
			}
			width_sum += layout.margin_right;
		}
	}
	width_sum += layout.padding_right;
	int rest_width = width - width_sum;
	//Layout horizontally
	int curr_x = layout.padding_top;
	for (Node* node : get_children()) {
		if (node->is_visible()) {
			NodeLayout& layout = node->get_layout();
			//Position
			curr_x += layout.margin_left;
			float factor = weight_sum == 0 ? 0 : (float) layout.x_weight/weight_sum;
			node->update_layout(round(rest_width * factor), height - this->layout.padding_top - this->layout.padding_bottom);
			//Alignment
			int y = 0;
			switch (layout.valignment) {
			case VerticalAlignment::TOP:
				y = this->layout.padding_top;
				break;
			case VerticalAlignment::CENTER:
				y = height/2 - node->get_height()/2;
				break;
			case VerticalAlignment::BOTTOM:
				y = height - this->layout.padding_bottom - height;
				break;
			}
			node->update_position(curr_x, y);
			curr_x += node->get_width();
			curr_x += layout.margin_right;
		}
	}
}

void HBox::draw(int parentX, int parentY, NodeEnv env) {
	DrawRectangle(parentX + x, parentY + y, width, height, style.fill_color);
	Parent::draw(parentX, parentY, env);
}

Vector HBox::get_content_size() {
	int curr_x = 0;
	int curr_y = 0;
	for (Node* node : get_children()) {
		if(node->is_visible()) {
			Vector size = node->calc_size(0, 0, true);
			curr_x += node->get_layout().margin_left;
			curr_x += size.x;
			curr_x += node->get_layout().margin_right;

			if (curr_y < size.y) {
				curr_y = size.y;
			}
		}
	}
	return {curr_x, curr_y};
}
