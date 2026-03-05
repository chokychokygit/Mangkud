#pragma once

namespace Mangkudd {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for camera
	/// </summary>
	public ref class camera : public System::Windows::Forms::Form
	{
	public:
		camera(void)
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
		~camera()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ToolStripContainer^ toolStripContainer1;
	private: System::Windows::Forms::SplitContainer^ splitContainer1;
	private: System::Windows::Forms::PictureBox^ camerashow;
	private: System::Windows::Forms::Button^ stopbutton;

	private: System::Windows::Forms::Button^ startbutton;
	private: System::Windows::Forms::Button^ button1;

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
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(camera::typeid));
			this->toolStripContainer1 = (gcnew System::Windows::Forms::ToolStripContainer());
			this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
			this->camerashow = (gcnew System::Windows::Forms::PictureBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->stopbutton = (gcnew System::Windows::Forms::Button());
			this->startbutton = (gcnew System::Windows::Forms::Button());
			this->toolStripContainer1->ContentPanel->SuspendLayout();
			this->toolStripContainer1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->BeginInit();
			this->splitContainer1->Panel1->SuspendLayout();
			this->splitContainer1->Panel2->SuspendLayout();
			this->splitContainer1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->camerashow))->BeginInit();
			this->SuspendLayout();
			// 
			// toolStripContainer1
			// 
			// 
			// toolStripContainer1.ContentPanel
			// 
			this->toolStripContainer1->ContentPanel->BackColor = System::Drawing::SystemColors::ActiveCaption;
			this->toolStripContainer1->ContentPanel->Controls->Add(this->splitContainer1);
			this->toolStripContainer1->ContentPanel->Size = System::Drawing::Size(1019, 644);
			this->toolStripContainer1->ContentPanel->Load += gcnew System::EventHandler(this, &camera::toolStripContainer1_ContentPanel_Load);
			this->toolStripContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->toolStripContainer1->Location = System::Drawing::Point(0, 0);
			this->toolStripContainer1->Name = L"toolStripContainer1";
			this->toolStripContainer1->Size = System::Drawing::Size(1019, 669);
			this->toolStripContainer1->TabIndex = 0;
			this->toolStripContainer1->Text = L"toolStripContainer1";
			// 
			// splitContainer1
			// 
			this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitContainer1->Location = System::Drawing::Point(0, 0);
			this->splitContainer1->Name = L"splitContainer1";
			// 
			// splitContainer1.Panel1
			// 
			this->splitContainer1->Panel1->Controls->Add(this->camerashow);
			// 
			// splitContainer1.Panel2
			// 
			this->splitContainer1->Panel2->Controls->Add(this->button1);
			this->splitContainer1->Panel2->Controls->Add(this->stopbutton);
			this->splitContainer1->Panel2->Controls->Add(this->startbutton);
			this->splitContainer1->Size = System::Drawing::Size(1019, 644);
			this->splitContainer1->SplitterDistance = 804;
			this->splitContainer1->TabIndex = 0;
			// 
			// camerashow
			// 
			this->camerashow->BackColor = System::Drawing::SystemColors::ActiveCaption;
			this->camerashow->Dock = System::Windows::Forms::DockStyle::Fill;
			this->camerashow->Location = System::Drawing::Point(0, 0);
			this->camerashow->Name = L"camerashow";
			this->camerashow->Size = System::Drawing::Size(804, 644);
			this->camerashow->TabIndex = 0;
			this->camerashow->TabStop = false;
			this->camerashow->Click += gcnew System::EventHandler(this, &camera::camerashow_Click);
			// 
			// button1
			// 
			this->button1->Font = (gcnew System::Drawing::Font(L"Microsoft YaHei", 16.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button1->Location = System::Drawing::Point(25, 429);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(161, 80);
			this->button1->TabIndex = 6;
			this->button1->Text = L"RETURN";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &camera::button1_Click);
			// 
			// stopbutton
			// 
			this->stopbutton->BackColor = System::Drawing::Color::Brown;
			this->stopbutton->Enabled = false;
			this->stopbutton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->stopbutton->ForeColor = System::Drawing::SystemColors::ButtonFace;
			this->stopbutton->Location = System::Drawing::Point(47, 256);
			this->stopbutton->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->stopbutton->Name = L"stopbutton";
			this->stopbutton->Size = System::Drawing::Size(108, 81);
			this->stopbutton->TabIndex = 4;
			this->stopbutton->Text = L"Stop";
			this->stopbutton->UseVisualStyleBackColor = false;
			// 
			// startbutton
			// 
			this->startbutton->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->startbutton->BackColor = System::Drawing::SystemColors::ActiveBorder;
			this->startbutton->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Zoom;
			this->startbutton->ForeColor = System::Drawing::SystemColors::ActiveCaption;
			this->startbutton->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"startbutton.Image")));
			this->startbutton->Location = System::Drawing::Point(47, 104);
			this->startbutton->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->startbutton->Name = L"startbutton";
			this->startbutton->Size = System::Drawing::Size(108, 83);
			this->startbutton->TabIndex = 1;
			this->startbutton->UseVisualStyleBackColor = false;
			this->startbutton->Click += gcnew System::EventHandler(this, &camera::startbutton_Click);
			// 
			// camera
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1019, 669);
			this->Controls->Add(this->toolStripContainer1);
			this->Name = L"camera";
			this->Text = L"camera";
			this->Load += gcnew System::EventHandler(this, &camera::camera_Load);
			this->toolStripContainer1->ContentPanel->ResumeLayout(false);
			this->toolStripContainer1->ResumeLayout(false);
			this->toolStripContainer1->PerformLayout();
			this->splitContainer1->Panel1->ResumeLayout(false);
			this->splitContainer1->Panel2->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->EndInit();
			this->splitContainer1->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->camerashow))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void toolStripContainer1_ContentPanel_Load(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void roiButton_Click(System::Object^ sender, System::EventArgs^ e) {
	}
private: System::Void startbutton_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void camerashow_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void camera_Load(System::Object^ sender, System::EventArgs^ e) {
}
};
}
