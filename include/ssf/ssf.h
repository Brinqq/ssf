namespace ssf{

enum SystemType{
    graphics = 0x0,
    input    = 0x2,
    compute  = 0x4,
    UILayer  = 0x8
};

int Init(const SystemType sys);
int Init();
int SystemsCreate(const SystemType sys);

}//namespace ssf
