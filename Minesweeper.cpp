#include <vector>
#include <chrono>
#include <random>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream> 
#include <cmath>
using namespace std;
using namespace sf;
void draw_text(bool i_black, unsigned short i_x, unsigned short i_y, const string& i_text, RenderWindow& i_window) { //hàm vẽ một chuỗi văn bản lên cửa sổ trò chơi
	short character_x = i_x;
	short character_y = i_y;
	unsigned char character_width;
	Sprite character_sprite;
	Texture font_texture;
	font_texture.loadFromFile("Resources/Images/Font.png"); //tải phông chữ từ tập file ảnh
	character_width = font_texture.getSize().x / 96; //tính toán chiều rộng của mỗi ký tự
	character_sprite.setTexture(font_texture);
	if (1 == i_black) { //nếu i_black là true, thiết lập màu của sprite thành đen
		character_sprite.setColor(Color(0, 0, 0));
	}
	for (const char a : i_text) { //Vòng lặp qua từng ký tự trong i_text, đặt vị trí và vẽ ký tự đó lên cửa sổ
		if ('\n' == a) {
			character_x = i_x;
			character_y += font_texture.getSize().y;

			continue;
		}
		character_sprite.setPosition(character_x, character_y);
		character_sprite.setTextureRect(IntRect(character_width * (a - 32), 0, character_width, font_texture.getSize().y));
		character_x += character_width;
		i_window.draw(character_sprite);
	}
}
class Cell{
	bool flags;  
	bool mines;  
	bool is_open;  
	unsigned char effect_timer;  
	unsigned char mines_around;  
	unsigned char mouse_state;   
	unsigned char x; 
	unsigned char y;
public:
	Cell(unsigned char i_x, unsigned char i_y);  //khởi tạo ô với tọa độ x, y
	//trả về các giá trị thuộc tính của ô
	bool get_flags();
	bool get_mines();
	bool get_is_open();
	bool open(vector<Cell>& i_cells);  //Mở ô và đệ quy mở các ô lân cận nếu ô này không có mìn xung quanh
	bool update_effect_timer();  //cập nhập bộ đếm thời gian hiệu ứng và trả về nếu thời gian hiệu ứng đã kết thúc
	unsigned char get_effect_timer();
	unsigned char get_mines_around();
	unsigned char get_mouse_state();
	void count_mines_around(vector<Cell>& i_cells);//Đếm số lượng mìn trong các ô lân cận
	void flag();
	void reset(); //đặt ô về trạng thái đầu
	void set_effect_timer(unsigned char i_effect_timer); //phương thức setter để thiết lập bộ đếm thời gian hiệu ứng
	void set_mine(); //phương thức setter để thiết lập trạng thái có mìn
	void set_mouse_state(unsigned char i_mouse_state); //phương thức setter để thiết lập trạng thái chuột
};
void Field::set_mouse_state(unsigned char i_mouse_state, unsigned char i_x, unsigned char i_y) { //thiết lập trạng thái chuột cho một ô cụ thể trên bảng
	get_cell(i_x, i_y, cells)->set_mouse_state(i_mouse_state);//lấy ô tại tọa độ (i_x, i_y) từ danh sách các ô rồi thiết lập trạng thái chuột mới cho ô đó.
}
class Field {
	bool first_click;
	char game_over;
	default_random_engine random_engine;
	random_device random_device;
	vector<Cell> cells;
public:
	Field();
	bool effect_over();
	char get_game_over();
	unsigned short get_flags();
	void draw(RenderWindow& i_window); //vẽ cửa sổ
	void flag_cell(unsigned char i_x, unsigned char i_y); //gắn cờ cho 1 ô
	void open_cell(unsigned char i_x, unsigned char i_y); //mở ô
	void restart(); //khởi động lại trod chơi
	void set_mouse_state(unsigned char i_mouse_state, unsigned char i_x, unsigned char i_y); //thiết lập trạng thái chuột cho một ô
};
template <typename T> //khai báo template cho phép hàm clamp hoạt động
const T& clamp(const T& v, const T& lo, const T& hi) { 
	return (v < lo) ? lo : (hi < v) ? hi : v; //Hàm clamp đảm bảo giá trị cần kiểm tra v nằm trong khoảng từ lo đến hi
}
constexpr unsigned char CELL_SIZE = 16; //kích thước ô
constexpr unsigned char COLUMNS = 8;
constexpr unsigned char EFFECT_DURATION = 16; //thời gian hiệu ứng hình ảnh
constexpr unsigned char FONT_HEIGHT = 16;  //kích thước phông chữ 
constexpr unsigned char FONT_WIDTH = 8;
constexpr unsigned char ROWS = 8;
constexpr unsigned char SCREEN_RESIZE = 16;  //tham số kích thước màn hình
constexpr unsigned short FRAME_DURATION = 16667;
constexpr unsigned short MINES = 8; //số lượng mìn trong tò chơi
void draw_text(bool i_black, unsigned short i_x, unsigned short i_y, const string& i_text, RenderWindow& i_window); //vẽ văn bản trên cửa sổ trò chơi
Cell::Cell(unsigned char i_x, unsigned char i_y) :	mines_around(0), x(i_x), y(i_y){  //khởi tạo vị trí ô và đặt số mìn xung quanh là 0
	reset();
}
//định nghĩa các phương thức lấy giá trị thuộc tính
bool Cell::get_flags(){
	return flags;
}
bool Cell::get_mines(){
	return mines;
}
bool Cell::get_is_open(){
	return is_open;
}
Cell* get_cell(unsigned char i_x, unsigned char i_y, vector<Cell>& i_cells) { //tính toán vị trí của ô trong vector i_cell và trả về con trỏ tới ô đó
	return &i_cells[i_x + COLUMNS * i_y];
}
bool Cell::open(vector<Cell>& i_cells){
	if (0 == is_open){ //mở ô nếu ô chưa mở
		is_open = 1;
		if (0 == mines && 0 == mines_around){ //nếu ô không chứa mìn và không có mìn xung quanh
			for (char a = -1; a < 2; a++){
				for (char b = -1; b < 2; b++){
					if ((0 == a && 0 == b) || (0 > a + x || 0 > b + y || COLUMNS == a + x || ROWS == b + y)){
						continue;
					}
					get_cell(a + x, b + y, i_cells)->open(i_cells); //đệ quy mở các ô lân cận
				}
			}
		}
		effect_timer -= mines;
		return mines;
	}
	return 0;
}
bool Cell::update_effect_timer(){ //giảm bộ đếm thời gian hiệu ứng và trả về true nếu bộ đếm đã đạt đến không
	if (0 < effect_timer){
		effect_timer--;

		if (0 == effect_timer){
			return 1;
		}
	}
	return 0;
}
unsigned char Cell::get_effect_timer(){
	return effect_timer;
}
unsigned char Cell::get_mines_around(){
	return mines_around;
}
unsigned char Cell::get_mouse_state(){
	return mouse_state;
}
void Cell::count_mines_around(vector<Cell>& i_cells){  //đếm số mìn xung quanh
	mines_around = 0;
	if (0 == mines){
		for (char a = -1; a < 2; a++){
			for (char b = -1; b < 2; b++){
				if ((0 == a && 0 == b) || (0 > a + x || 0 > b + y || COLUMNS == a + x || ROWS == b + y)){
					continue;
				}
				if (1 == get_cell(a + x, b + y, i_cells)->get_mines()){
					mines_around++; 
				}
			}
		}
	}
}
void Cell::flag(){ //chuyển đổi trạng thái gắn cờ nếu ô chưa mở
	if (0 == is_open){
		flags = 1 - flags;
	}
}
void Cell::reset(){ //đặt lại các thuộc tính của ô về trạng thái ban đầu
	flags = 0;
	mines = 0;
	is_open = 0;
	effect_timer = EFFECT_DURATION;
	mouse_state = 0;
}
void Cell::set_effect_timer(unsigned char i_effect_timer){ //thiết lập bộ đếm thời gian hiệu ứng
	effect_timer = i_effect_timer;
}
void Cell::set_mine(){ //Thiết lập trạng thái có mìn
	mines = 1;
}
void Cell::set_mouse_state(unsigned char i_mouse_state){ //Thiết lập trạng thái chuột
	mouse_state = i_mouse_state;
}
Field::Field() : game_over(1), random_engine(random_device()){
	for (unsigned char a = 0; a < ROWS; a++){
		for (unsigned char b = 0; b < COLUMNS; b++){
			cells.push_back(Cell(b, a));
		}
	}
	restart();
}
bool Field::effect_over(){ //kiểm tra nếu tất cả các hiệu ứng hình ảnh đã kết thúc. Nếu bất kỳ ô nào còn thời gian hiệu ứng, trả về false, ngược lại trả về true
	for (Cell& cell : cells){
		if (0 < cell.get_effect_timer()){
			return 0;
		}
	}
	return 1;
}
char Field::get_game_over(){ //trả về trạng thái kết thúc của trò chơi
	return game_over;
}
unsigned short Field::get_flags(){  //đếm và trả về số lượng ô đã được gắn cờ
	unsigned short total_flags = 0;
	for (Cell& cell : cells){
		total_flags += cell.get_flags();
	}
	return total_flags;
}
void Field::draw(RenderWindow& i_window){
	RectangleShape cell_shape(Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));  //hình dạng vẽ ô
	Sprite icon_sprite;  //vẽ các ký hiệu/ký tự trên ô
	Texture icons_texture;
	icons_texture.loadFromFile("Resources/Images/Icons" + to_string(CELL_SIZE) + ".png");
	icon_sprite.setTexture(icons_texture);
	//Vòng lặp vẽ các ô, vòng lặp này đi qua tất cả các ô trong bảng
	for (unsigned char a = 0; a < COLUMNS; a++){
		for (unsigned char b = 0; b < ROWS; b++){
			cell_shape.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b)); //vị trí của ô được thiết lập dựa trên tọa độ của ô hiện tại.
			if (1 == get_cell(a, b, cells)->get_is_open()){ //Kiểm tra trạng thái của ô
				unsigned char mines_around = get_cell(a, b, cells)->get_mines_around();
				cell_shape.setFillColor(Color(146, 182, 255)); //ô được tô màu xanh nhạt nếu ô mở 
				i_window.draw(cell_shape);
				if (0 < mines_around){
					icon_sprite.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));
					icon_sprite.setTextureRect(IntRect(CELL_SIZE * mines_around, 0, CELL_SIZE, CELL_SIZE)); //Cắt phần hình ảnh tương ứng với số lượng mìn xung quanh từ tập tin hình ảnh
					i_window.draw(icon_sprite);  //vẽ biểu tượng số lượng mìn xung quanh
				}
			}else {
				cell_shape.setFillColor(Color(0, 73, 255)); //màu xanh đậm cho ô chưa mở
				if (0 == game_over){
					if (1 == get_cell(a, b, cells)->get_mouse_state()){
						cell_shape.setFillColor(Color(36, 109, 255)); //màu xanh nhạt hơn khi trạng thái chuột di chuyển đến 
					}
					else if (2 == get_cell(a, b, cells)->get_mouse_state()){
						cell_shape.setFillColor(Color(0, 36, 255));//màu xanh đậm hơn khi ô được nhấn bởi chuột
					}
				}
				i_window.draw(cell_shape);
				if (1 == get_cell(a, b, cells)->get_flags()){
					icon_sprite.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));
					icon_sprite.setTextureRect(IntRect(0, 0, CELL_SIZE, CELL_SIZE));
					i_window.draw(icon_sprite);//vẽ biểu tượng gắn cờ
				}
			}
			get_cell(a, b, cells)->set_mouse_state(0);
			if (0 != game_over && EFFECT_DURATION > get_cell(a, b, cells)->get_effect_timer()){//nếu trò chơi đã kết thúc và thời gian hiệu ứng còn lại hiệu ứng sẽ được vẽ
				unsigned char effect_size = static_cast<unsigned char>(2 * round(0.5f * CELL_SIZE * ((EFFECT_DURATION - get_cell(a, b, cells)->get_effect_timer()) / static_cast<float>(EFFECT_DURATION))));
				uniform_int_distribution<unsigned short> effect_duration_distribution(1, EFFECT_DURATION - 1);
				cell_shape.setPosition(floor(CELL_SIZE * (0.5f + a) - 0.5f * effect_size), floor(CELL_SIZE * (0.5f + b) - 0.5f * effect_size));
				cell_shape.setSize(Vector2f(effect_size, effect_size));
				if (-1 == game_over){
					cell_shape.setFillColor(Color(255, 36, 0));//màu đỏ cho hiệu ứng kết thúc trò chơi khi thua
				}
				else{
					cell_shape.setFillColor(Color(255, 255, 255));//màu trắng cho hiệu ứng kết thúc trò chơi khi thắng
				}
				i_window.draw(cell_shape);
				cell_shape.setSize(Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
				//nếu bộ đếm thời gian của ô đã kết thúc (update_effect_timer() trả về true) bộ đếm thời gian của các ô lân cận sẽ được thiết lập lại ngẫu nhiên.
				if (1 == get_cell(a, b, cells)->update_effect_timer()){ 
					if (0 <= a - 1 && EFFECT_DURATION == get_cell(a - 1, b, cells)->get_effect_timer()){
						get_cell(a - 1, b, cells)->set_effect_timer(static_cast<unsigned char>(effect_duration_distribution(random_engine)));
					}

					if (0 <= b - 1 && EFFECT_DURATION == get_cell(a, b - 1, cells)->get_effect_timer()){
						get_cell(a, b - 1, cells)->set_effect_timer(static_cast<unsigned char>(effect_duration_distribution(random_engine)));
					}

					if (COLUMNS > 1 + a && EFFECT_DURATION == get_cell(1 + a, b, cells)->get_effect_timer()){
						get_cell(1 + a, b, cells)->set_effect_timer(static_cast<unsigned char>(effect_duration_distribution(random_engine)));
					}

					if (ROWS > 1 + b && EFFECT_DURATION == get_cell(a, 1 + b, cells)->get_effect_timer()){
						get_cell(a, 1 + b, cells)->set_effect_timer(static_cast<unsigned char>(effect_duration_distribution(random_engine)));
					}
				}
			}
		}
	}
}
void Field::flag_cell(unsigned char i_x, unsigned char i_y){ //gắn cờ cho một ô nếu trò chơi chưa kết thúc (game_over == 0)
	if (0 == game_over){
		get_cell(i_x, i_y, cells)->flag(); //gắn hoặc bỏ cờ
	}
}
void Field::open_cell(unsigned char i_x, unsigned char i_y){
	if (0 == first_click){ //Kiểm tra lần nhấp đầu tiên
		uniform_int_distribution<unsigned short> x_distribution(0, COLUMNS - 1);
		uniform_int_distribution<unsigned short> y_distribution(0, ROWS - 1);
		first_click = 1;
		for (unsigned short a = 0; a < MINES; a++){ //đặt mìn ngẫu nhiên
			unsigned char mine_x = static_cast<unsigned char>(x_distribution(random_engine));
			unsigned char mine_y = static_cast<unsigned char>(y_distribution(random_engine));
			if (1 == get_cell(mine_x, mine_y, cells)->get_mines() || (i_x == mine_x && i_y == mine_y)){ //Nếu vị trí đã có mìn hoặc trùng với vị trí ô đầu tiên nhấp, vòng lặp sẽ lặp lại (a--)
				a--;
			}
			else{
				get_cell(mine_x, mine_y, cells)->set_mine();
			}
		}
		for (Cell& cell : cells){ //đếm số mìn xung quanh các ô và lưu trữ kết quả trong mỗi ô
			cell.count_mines_around(cells);
		}
	}
	if (0 == game_over && 0 == get_cell(i_x, i_y, cells)->get_flags()){//Kiểm tra trạng thái trò chơi và đảm bảo ô chưa được gắn cờ trước khi mở
		if (1 == get_cell(i_x, i_y, cells)->open(cells)){ //nếu ô chứa mìn, trò chơi kết thúc
			game_over = -1;
		}
		else{ //nếu ô không chứa mìn, phương thức kiểm tra xem còn bao nhiêu ô chưa mở 
			unsigned short total_closed_cells = 0;
			for (Cell& cell : cells){
				total_closed_cells += 1 - cell.get_is_open();
			}
			if (MINES == total_closed_cells){//Nếu số ô chưa mở bằng số lượng mìn, trò chơi kết thúc với chiến thắng
				game_over = 1;
				get_cell(i_x, i_y, cells)->set_effect_timer(EFFECT_DURATION - 1);
			}
		}
	}
}
void Field::restart(){ //khởi động lại trò chơi
	if (0 != game_over){
		first_click = 0;
		game_over = 0;
		for (Cell& cell : cells){
			cell.reset();
		}
	}
}
int main(){
	unsigned lag = 0; //biến lưu trữ độ trễ khung hình 
	unsigned char icons_state = 0; //biến lưu trữ trạng thái icon
	RenderWindow window(VideoMode(800, 600), "Clamp Example"); //cửa sổ trò chơi được khởi tạo với kích thước dựa trên các hằng số
	chrono::time_point<chrono::steady_clock> previous_time; //tính toán thời gian trôi qua giữa các khung hình
	Event event; //xử lý sưj kiện từ user 
	RenderWindow window(VideoMode(CELL_SIZE * COLUMNS * SCREEN_RESIZE, SCREEN_RESIZE * (FONT_HEIGHT + CELL_SIZE * COLUMNS)), "Minesweeper", Style::Close); //cửa sổ đầu tiên có kích thước 800x600 chỉ để kiểm tra, sau đó được tạo lại với kích thước phù hợp cho trò chơi Minesweeper
	window.setView(View(FloatRect(0, 0, CELL_SIZE * COLUMNS, FONT_HEIGHT + CELL_SIZE * ROWS)));
	Sprite icons;
	Texture icons_texture;
	icons_texture.loadFromFile("Resources/Images/Alexander.png");
	icons.setPosition(static_cast<float>(CELL_SIZE * COLUMNS - icons_texture.getSize().y), CELL_SIZE * ROWS); //vị trí icon
	icons.setTexture(icons_texture);
	Field field; //khởi tạo field để quản lý bảng Minesweeper
	previous_time = chrono::steady_clock::now(); //lưu trữ tg hiẹne tại
	while (1 == window.isOpen()){
		unsigned delta_time = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - previous_time).count();
		lag += delta_time;
		previous_time += chrono::microseconds(delta_time); 
		while (FRAME_DURATION <= lag){//Xử lý các sự kiện nếu độ trễ vượt quá thời gian khung hình
			unsigned char mouse_cell_x = clamp(static_cast<int>(floor(Mouse::getPosition(window).x / static_cast<float>(CELL_SIZE * SCREEN_RESIZE))), 0, COLUMNS - 1); //tọa độ của chuột trên bảng được tính và giới hạn bằng hàm `clamp`
			unsigned char mouse_cell_y = clamp(static_cast<int>(floor(Mouse::getPosition(window).y / static_cast<float>(CELL_SIZE * SCREEN_RESIZE))), 0, ROWS - 1);
			lag -= FRAME_DURATION; //Giảm độ trễ theo thời gian khung hình
			while (1 == window.pollEvent(event)){ //các thao tác 
				switch (event.type){
					case Event::Closed:{ //đóng cửa sổ khi người dùng yêu cầu
						window.close();
						break;
					}
					case Event::KeyReleased:{
						switch (event.key.code){
							case Keyboard::Enter:{ //khởi động lại trò chơi khi nhấn phím Enter
								field.restart();
							}
						}
						break;
					}
					case Event::MouseButtonReleased:{
						switch (event.mouseButton.button) {
						case Mouse::Left: {  //mở ô khi nhấn chuột trái
							field.open_cell(mouse_cell_x, mouse_cell_y);
							break;
						}
						case Mouse::Right: {  //gắn cờ cho ô khi nhấn chuột phải
							field.flag_cell(mouse_cell_x, mouse_cell_y);
						}
						}
					}
				}
			}
		}
		//cập nhập trạng thái của icon cảm xúc
		if (1 == Mouse::isButtonPressed(Mouse::Left) || 1 == Mouse::isButtonPressed(Mouse::Right)){ //nếu nhấn chuột trái/phải icon 1
			icons_state = 1;
		} else{
			icons_state = 0;
		}
		if (-1 == field.get_game_over()){ //nếu thua icon 2
			icons_state = 2;
		}
		else if (1 == field.get_game_over()){ //nếu thắng trạng thái 3
			icons_state = 3;
		}
		if (FRAME_DURATION > lag){ //nếu `FRAME_DURATION` lớn hơn `lag`, cửa sổ được xóa và bảng Minesweeper được vẽ
			window.clear();
			field.draw(window); //Vẽ bảng Minesweeper
			if (1 == field.effect_over()){
				if (1 == field.get_game_over()){
					draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS - 8 * FONT_WIDTH))), static_cast<unsigned short>(round(0.5f * (CELL_SIZE * ROWS - FONT_HEIGHT))), "VICTORY!", window);	//hiển thị khi thắng		
				} else{
					draw_text(1, static_cast<unsigned short>(round(0.5f * (CELL_SIZE * COLUMNS - 4 * FONT_WIDTH))), static_cast<unsigned short>(round(0.5f * (CELL_SIZE * ROWS - 2 * FONT_HEIGHT))), "GAME\nOVER", window);  //hiển thi khi thua
				}
			}
			draw_text(0, 0, CELL_SIZE * ROWS, "Mines:" + to_string(MINES - field.get_flags()), window); //hiển thị số lượng mìn còn lại cần gắn cờ
			icons.setTextureRect(IntRect(icons_state * icons_texture.getSize().y, 0, icons_texture.getSize().y, icons_texture.getSize().y));
			window.draw(icons);
			window.display();
		}
	}
}
