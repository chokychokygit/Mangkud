#pragma once

namespace Mangkudd {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for showvdo
	/// </summary>
	public ref class showvdo : public System::Windows::Forms::Form
	{
	public:
		showvdo(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~showvdo()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ToolStripContainer^ toolStripContainer1;
	private: System::Windows::Forms::Button^ pusebutton;
	protected:

	private: System::Windows::Forms::Button^ button1;
	private: System::Windows::Forms::PictureBox^ pictureBox1;
	private: System::Windows::Forms::TextBox^ gradeA;
	private: System::Windows::Forms::TextBox^ textBox2;
	private: System::Windows::Forms::TextBox^ textBox3;
	private: System::Windows::Forms::TextBox^ textBox1;


	protected:



	protected:

	protected:

	protected:


	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(showvdo::typeid));
			this->toolStripContainer1 = (gcnew System::Windows::Forms::ToolStripContainer());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->pusebutton = (gcnew System::Windows::Forms::Button());
			this->gradeA = (gcnew System::Windows::Forms::TextBox());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->textBox2 = (gcnew System::Windows::Forms::TextBox());
			this->textBox3 = (gcnew System::Windows::Forms::TextBox());
			this->toolStripContainer1->ContentPanel->SuspendLayout();
			this->toolStripContainer1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			this->SuspendLayout();
			// 
			// toolStripContainer1
			// 
			// 
			// toolStripContainer1.ContentPanel
			// 
			this->toolStripContainer1->ContentPanel->BackColor = System::Drawing::SystemColors::ActiveCaption;
			this->toolStripContainer1->ContentPanel->Controls->Add(this->textBox2);
			this->toolStripContainer1->ContentPanel->Controls->Add(this->textBox3);
			this->toolStripContainer1->ContentPanel->Controls->Add(this->textBox1);
			this->toolStripContainer1->ContentPanel->Controls->Add(this->gradeA);
			this->toolStripContainer1->ContentPanel->Controls->Add(this->pusebutton);
			this->toolStripContainer1->ContentPanel->Controls->Add(this->button1);
			this->toolStripContainer1->ContentPanel->Controls->Add(this->pictureBox1);
			this->toolStripContainer1->ContentPanel->Size = System::Drawing::Size(989, 634);
			this->toolStripContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->toolStripContainer1->Location = System::Drawing::Point(0, 0);
			this->toolStripContainer1->Name = L"toolStripContainer1";
			this->toolStripContainer1->Size = System::Drawing::Size(989, 634);
			this->toolStripContainer1->TabIndex = 1;
			this->toolStripContainer1->Text = L"toolStripContainer1";
			// 
			// pictureBox1
			// 
			this->pictureBox1->Location = System::Drawing::Point(12, 84);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(766, 538);
			this->pictureBox1->TabIndex = 0;
			this->pictureBox1->TabStop = false;
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(34, 12);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(162, 66);
			this->button1->TabIndex = 1;
			this->button1->Text = L"Return";
			this->button1->UseVisualStyleBackColor = true;
			// 
			// pusebutton
			// 
			this->pusebutton->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"pusebutton.BackgroundImage")));
			this->pusebutton->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Zoom;
			this->pusebutton->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"pusebutton.Image")));
			this->pusebutton->Location = System::Drawing::Point(458, 12);
			this->pusebutton->Name = L"pusebutton";
			this->pusebutton->Size = System::Drawing::Size(75, 66);
			this->pusebutton->TabIndex = 2;
			this->pusebutton->UseVisualStyleBackColor = true;
			this->pusebutton->Click += gcnew System::EventHandler(this, &showvdo::pusebutton_Click);
			// 
			// gradeA
			// 
			this->gradeA->BackColor = System::Drawing::SystemColors::ControlDark;
			this->gradeA->Font = (gcnew System::Drawing::Font(L"Goudy Old Style", 28.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->gradeA->Location = System::Drawing::Point(784, 109);
			this->gradeA->Name = L"gradeA";
			this->gradeA->Size = System::Drawing::Size(193, 64);
			this->gradeA->TabIndex = 3;
			this->gradeA->Text = L"A\r\n";
			// 
			// textBox1
			// 
			this->textBox1->BackColor = System::Drawing::SystemColors::ControlDark;
			this->textBox1->Font = (gcnew System::Drawing::Font(L"Goudy Old Style", 28.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox1->Location = System::Drawing::Point(784, 230);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(193, 64);
			this->textBox1->TabIndex = 4;
			this->textBox1->Text = L"B";
			// 
			// textBox2
			// 
			this->textBox2->BackColor = System::Drawing::SystemColors::ControlDark;
			this->textBox2->Font = (gcnew System::Drawing::Font(L"Goudy Old Style", 28.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox2->Location = System::Drawing::Point(784, 458);
			this->textBox2->Name = L"textBox2";
			this->textBox2->Size = System::Drawing::Size(193, 64);
			this->textBox2->TabIndex = 6;
			this->textBox2->Text = L"D";
			// 
			// textBox3
			// 
			this->textBox3->BackColor = System::Drawing::SystemColors::ControlDark;
			this->textBox3->Font = (gcnew System::Drawing::Font(L"Goudy Old Style", 28.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox3->Location = System::Drawing::Point(784, 347);
			this->textBox3->Name = L"textBox3";
			this->textBox3->Size = System::Drawing::Size(193, 64);
			this->textBox3->TabIndex = 5;
			this->textBox3->Text = L"C";
			// 
			// showvdo
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(989, 634);
			this->Controls->Add(this->toolStripContainer1);
			this->Name = L"showvdo";
			this->Text = L"showvdo";
			this->Load += gcnew System::EventHandler(this, &showvdo::showvdo_Load);
			this->toolStripContainer1->ContentPanel->ResumeLayout(false);
			this->toolStripContainer1->ContentPanel->PerformLayout();
			this->toolStripContainer1->ResumeLayout(false);
			this->toolStripContainer1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void showvdo_Load(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void textBox1_TextChanged(System::Object^ sender, System::EventArgs^ e) {
	}

	private: System::Void pusebutton_Click(System::Object^ sender, System::EventArgs^ e) {
	}
};
}
