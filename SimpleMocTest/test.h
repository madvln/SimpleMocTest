#pragma once

TEST(MOC_Solver, TemplatedLayer1)
{

    profile_collection_t<1> tl1(10); // 1 ������� � ������� ����������� 10
    profile_collection_t<2> tl2(10); // 2 ������� � ������� ����������� 10

    profile_collection_t<0, 1> tl_cell1(10); // 1 ������� � �������� ����������� 9 (10 ����� = 9 �����)
    profile_collection_t<0, 2> tl_cell2(10); // 2 ������� � �������� ����������� 9 (10 ����� = 9 �����)
}

TEST(MOC_Solver, MOC_Layer)
{
    //���� ����������
    typedef profile_collection_t<1> var_layer;

    //���� ��������� ������ - 1 ����� ��� �����, 1 ����� ��� ������ 9 (��������� ������ ���-�)
    moc_solver<1>::specific_layer moc_layer(10);

    //���������� - ���� ���������� Vars + ������� ������ ��������� Specific
    composite_layer_t<var_layer,
        moc_solver<1>::specific_layer> composite_layer(10);

    //������� � ���������� ����, ������ �� ������� ������������ ����� composite_layer (Var+Specific)
    custom_buffer_t<composite_layer_t<profile_collection_t<1>, moc_solver<1>::specific_layer>> buffer(2, 10);

    //��������� ��������/����������� ����
    const composite_layer_t<var_layer, moc_solver<1>::specific_layer>& prev = buffer.previous();
    composite_layer_t<var_layer, moc_solver<1>::specific_layer>& next = buffer.current();

    //�������� �� ���� ������ 
    buffer.advance(+1);
}

TEST(MOC_Solver, MOC_Layer_Refactor)
{
    // ������� ����������
    typedef profile_collection_t<1> target_var_t;

    // ����: ���������� Vars + ������� ������ ��������� Specific
    typedef composite_layer_t<target_var_t, moc_solver<1>::specific_layer> layer_t;

    //������� � ���������� ����, ������ �� ������� ������������ ����� composite_layer (Var+Specific)
    custom_buffer_t<layer_t> buffer(2, 10);

    //��������� ��������/����������� ����
    const layer_t& prev = buffer.previous();
    layer_t& next = buffer.current();

    //�������� �� ���� ������ 
    buffer.advance(+1);
}


/// @brief ������� ������ ������������� ������ ������������� ��� ��������� ��������
TEST(MOC_Solver, UseCase_Advection)
{
    // ���������� ������� ����� - 50��, � ����� ��������� ��� �������� ����� 1��, ��������� 700��
    simple_pipe_properties simple_pipe;
    simple_pipe.length = 50e3;
    simple_pipe.diameter = 0.7;
    simple_pipe.dx = 1000;

    PipeProperties pipe = PipeProperties::build_simple_pipe(simple_pipe);

    // ���� ����������, � ��������� ������ ������������� ��� ���m
    typedef composite_layer_t<profile_collection_t<1>,
        moc_solver<1>::specific_layer> single_var_moc_t;

    custom_buffer_t<single_var_moc_t> buffer(2, pipe.profile.getPointCount());

    buffer.advance(+1);
    single_var_moc_t& prev = buffer.previous();
    single_var_moc_t& next = buffer.current();
    auto& rho_initial = prev.vars.point_double[0];
    rho_initial = vector<double>(rho_initial.size(), 850); // ������������� ��������� ���������

    vector<double> Q(pipe.profile.getPointCount(), 0.5); // ������ �� ����� ������ 0.5 �3/�
    PipeQAdvection advection_model(pipe, Q);

    moc_solver<1> solver(advection_model, prev, next);

    double dt = solver.prepare_step();
    double rho_in = 840; // ��������� �����, ������������ �� ����� ����� ��� ������������� �������
    double rho_out = 860; // ��������� �����, ������������ � ������ ����� ��� ������������� �������
    solver.step_optional_boundaries(dt, rho_in, rho_out);

    auto& c_new = next.vars.point_double[0];

    auto& c_old = prev.vars.point_double[0];
}


TEST(MOC_Solver, UseCase_Advection_Density_Sulfur_Buffer)
{
    // ���������� ������� ����� - 50��, � ����� ��������� ��� �������� ����� 1��, ��������� 700��
    simple_pipe_properties simple_pipe;
    simple_pipe.length = 50e3;
    simple_pipe.diameter = 0.7;
    simple_pipe.dx = 1000;

    PipeProperties pipe = PipeProperties::build_simple_pipe(simple_pipe);

    // ���� ����������, � ��������� ������ ������������� ��� ���
    typedef composite_layer_t<profile_collection_t<2>> density_sulfur_layer_t; //��� ������ ��� ���� ��������� � ����, 2 - ���������� ��������

    custom_buffer_t<density_sulfur_layer_t> buffer(2, pipe.profile.getPointCount()); //2 - ���������� �����, getPointCount - ����������� 
    // ������� ������� � ������ ����
    density_sulfur_layer_t& prev = buffer.previous();
    density_sulfur_layer_t& curr = buffer.current();
    vector<double>& rho_prev = prev.vars.point_double[0]; //0 - ������� ������� ���� prev
    rho_prev = vector<double>(rho_prev.size(), 850);
    vector<double>& rho_curr = curr.vars.point_double[0]; //0 - ������� ������� ���� curr
    rho_curr = vector<double>(rho_curr.size(), 860);

    buffer.advance(+1);
    density_sulfur_layer_t& prev2 = buffer.previous(); //prev2==curr, ��������� ������� ��� ���������� �������
    density_sulfur_layer_t& curr2 = buffer.current();  //curr2==prev, ��������� prev ������ �� �����, ��� ����� ������������ ��� ���. ����. �������
}

TEST(MOC_Solver, UseCase_Advection_Density_Sulfur_Calculation)
{
    // ���������� ������� ����� - 50��, � ����� ��������� ��� �������� ����� 1��, ��������� 700��
    simple_pipe_properties simple_pipe;
    simple_pipe.length = 50e3;
    simple_pipe.diameter = 0.7;
    simple_pipe.dx = 1000;

    PipeProperties pipe = PipeProperties::build_simple_pipe(simple_pipe);

    // ���� ����������, � ��������� ������ ������������� ��� ���
    typedef composite_layer_t<profile_collection_t<2>> density_sulfur_layer_t; //��� ������ ��� ���� ��������� � ����, 2 - ���������� ��������

    custom_buffer_t<density_sulfur_layer_t> buffer(2, pipe.profile.getPointCount()); //2 - ���������� �����, getPointCount - ����������� 
    // ������� ������� � ������ ����
    density_sulfur_layer_t& prev = buffer.previous();
    density_sulfur_layer_t& curr = buffer.current();

    vector<double>& rho_prev = prev.vars.point_double[0]; //0 - ������� ������� ���� prev
    rho_prev = vector<double>(rho_prev.size(), 850);
    vector<double>& sulfur_prev = prev.vars.point_double[1]; //1 - ������ ������� ���� prev
    sulfur_prev = vector<double>(sulfur_prev.size(), 1.55);

    vector<double>& rho_curr = curr.vars.point_double[0]; //0 - ������� ������� ���� prev
    rho_curr = vector<double>(rho_curr.size(), 0);
    vector<double>& sulfur_curr = curr.vars.point_double[1]; //1 - ������ ������� ���� prev
    sulfur_curr = vector<double>(sulfur_curr.size(), 0);


    //double rho_in = 840; // ��������� �����, ������������ �� ����� ����� ��� ������������� �������
    //double rho_out = 860; // ��������� �����, ������������ � ������ ����� ��� ������������� �������
    //double sulfur_in = 1.45; // ��������� �����, ������������ �� ����� ����� ��� ������������� �������
    //double sulfur_out = 1.65; // ��������� �����, ������������ � ������ ����� ��� ������������� �������

    //simple_moc_solver 
    int n = 51;//���������� �����
    int N = 5;//���-�� �����

    vector<double> flow(N, 0.5);
    vector<double> rho_in(N, 840);
    vector<double> rho_out(N, 860);
    vector<double> sulfur_in(N, 1.45);
    vector<double> sulfur_out(N, 1.65);
    vector<vector<double>> oil_in = { rho_in, sulfur_in };
    vector<vector<double>> oil_out = { rho_out, sulfur_out };

    ofstream fout_rho("layers_rho.txt");
    ofstream fout_sulfur("layers_sulfur.txt");
    for (int i = 0; i < N; i++)
    {
        if (flow[i] > 0)
        {

            for (int l = 0; l < n - 1; l++)
                rho_curr[l + 1] = rho_prev[l];
            rho_curr[0] = oil_in[0][N - 1 - i];

            for (int l = 0; l < n - 1; l++)
                sulfur_curr[l + 1] = sulfur_prev[l];
            sulfur_curr[0] = oil_in[1][N - 1 - i];
        }
        else
        {

            for (int l = n - 1; l > 0; l--)
                rho_curr[l - 1] = rho_prev[l];
            rho_curr[n - i] = oil_out[0][i];


            for (int l = n - 1; l > 0; l--)
                sulfur_curr[l - 1] = sulfur_prev[l];
            sulfur_curr[n - i] = oil_out[1][i];
        }



        if (i == 0)
        {
            for (int j = 0; j < n; j++)
            {
                fout_rho << rho_prev[j] << "\t";
                fout_sulfur << sulfur_prev[j] << "\t";
            }
            fout_rho << "\n";
            fout_sulfur << "\n";
        }
        for (int j = 0; j < n; j++)
        {
            fout_rho << rho_curr[j] << "\t";
            fout_sulfur << sulfur_curr[j] << "\t";
        }
        fout_rho << "\n";
        fout_sulfur << "\n";

        //rho_prev = rho_curr;
        //sulfur_prev = sulfur_curr;

        buffer.advance(+1);
        prev = buffer.previous();
        curr = buffer.current();
        rho_prev = prev.vars.point_double[0];
        rho_curr = curr.vars.point_double[0];
        sulfur_prev = prev.vars.point_double[1];
        sulfur_curr = curr.vars.point_double[1];
    }
    fout_rho.close();
    fout_sulfur.close();
    //density_sulfur_layer_t& prev2 = buffer.previous(); //prev2==curr, ��������� ������� ��� ���������� �������
    //density_sulfur_layer_t& curr2 = buffer.current();  //curr2==prev, ��������� prev ������ �� �����, ��� ����� ������������ ��� ���. ����. �������
}


/// @brief �����, ��������� ��� ������� ������������� ���������
class transport_equation_solver {
public:
    /// @brief ����������� �������, � ������� ���������� ������ �� ���������� ����, ������ �� ���� ����. ����������� �� ��������� ������, �� ���������� ������ �� �����
    /// @param pipe_len ����� ������� 
    /// @param step_len ����� ����
    /// @param flow ������
    /// @param layer_prev ���������� ����
    /// @param layer_curr ������� ����
    transport_equation_solver(double pipe_len, double step_len, double& flow, vector<double>& layer_prev, vector<double>& layer_curr)
        :layer_prev(layer_prev), layer_curr(layer_curr), flow(flow)
    {
        n = static_cast<int>(pipe_len / step_len + 0.5) + 1;//������� ���������� �����

    }
    /// @brief �����, �������� �������� ������, � ���� ���������� ��� ��������� �������
    /// @param oil_in ����� ��������� �������
    /// @param oil_out ������ ��������� �������
    void simple_step(double oil_in, double oil_out)
    {
        if (flow > 0)
        {
            for (int l = 0; l < n - 1; l++)
                layer_curr[l + 1] = layer_prev[l];
            layer_curr[0] = oil_in;
        }
        else
        {
            for (int l = n - 1; l > 0; l--)
                layer_curr[l - 1] = layer_prev[l];
            layer_curr[n] = oil_out;
        }
    }
    /// @brief ����� ��������� ���������� �����
    int get_point_count() const {
        return n;
    }
    /// @brief ����� ������ ����, ���� ������ - (����� � ���������)\pde_solvers\msvc
    /// @param layer_curr ���� ��� ������
    /// @param filename �������� �����
    void print_layers(vector<double> layer_curr, string filename) {
        ofstream fout(filename, ios::app);
        for (int j = 0; j < n; j++)
        {
            fout << layer_curr[j] << "\t";
        }
        fout << "\n";
    }

private:
    double& flow;
    int n;
    vector<double>& layer_prev;
    vector<double>& layer_curr;
};

TEST(MOC_Solver, UseCase_Advection_Density_Sulfur_Calculation_Class)
{
    // ���������� ������� ����� - 50��, � ����� ��������� ��� �������� ����� 1��, ��������� 700��
    simple_pipe_properties simple_pipe;
    simple_pipe.length = 50e3;
    simple_pipe.diameter = 0.7;
    simple_pipe.dx = 1000;

    double pipe_len = 50e3;
    //double step_len = simple_pipe.dx;

    PipeProperties pipe = PipeProperties::build_simple_pipe(simple_pipe);

    // ���� ����������, � ��������� ������ ������������� ��� ���
    typedef composite_layer_t<profile_collection_t<2>> density_sulfur_layer_t; //��� ������ ��� ���� ��������� � ����, 2 - ���������� ��������

    custom_buffer_t<density_sulfur_layer_t> buffer(2, pipe.profile.getPointCount()); //2 - ���������� �����, getPointCount - ����������� 
    // ������� ������� � ������ ����
    density_sulfur_layer_t& prev = buffer.previous();
    density_sulfur_layer_t& curr = buffer.current();

    vector<double>& rho_prev = prev.vars.point_double[0]; //0 - ������� ������� ���� prev
    rho_prev = vector<double>(rho_prev.size(), 850);
    vector<double>& sulfur_prev = prev.vars.point_double[1]; //1 - ������ ������� ���� prev
    sulfur_prev = vector<double>(sulfur_prev.size(), 1.55);
    vector<double>& rho_curr = curr.vars.point_double[0]; //0 - ������� ������� ���� prev
    rho_curr = vector<double>(rho_curr.size(), 850);
    vector<double>& sulfur_curr = curr.vars.point_double[1]; //1 - ������ ������� ���� prev
    sulfur_curr = vector<double>(sulfur_curr.size(), 1.55);

    double flow = 0.5;  //������ ������ �������������, ������� ������� �����
    double rho_in = 840;
    double rho_out = 860;
    double sulfur_in = 1.45;
    double sulfur_out = 1.65;
    string filename_rho = "layer_rho.txt";
    string filename_sulfur = "layer_sulfur.txt";



    transport_equation_solver simple_solver(simple_pipe.length, simple_pipe.dx, flow, rho_curr, rho_prev);
    simple_solver.simple_step(rho_in, rho_out);
    transport_equation_solver simple_solver(simple_pipe.length, simple_pipe.dx, flow, sulfur_curr, sulfur_prev);
    simple_solver.simple_step(sulfur_in, sulfur_out);
    buffer.advance(+1);

    simple_solver.print_layers(rho_curr, filename_rho);
    simple_solver.print_layers(sulfur_curr, filename_sulfur);

}


/// @brief ������ ��������� �������������, ����� ��������������� ������� �������������� ��������
/// ������� �������������
TEST(MOC_Solver, UseCase_Waterhammer)
{
    typedef profile_collection_t<2> layer_variables_type;
    typedef moc_solver<2>::specific_layer layer_moc_type;

    typedef composite_layer_t<layer_variables_type, layer_moc_type> composite_layer_type;

    custom_buffer_t<composite_layer_type> buffer(2, 3);

    PipeProperties pipe;
    pipe.profile.coordinates = { 0, 1000, 2000 };
    pipe.profile.heights = pipe.profile.capacity = vector<double>(pipe.profile.coordinates.size(), 0);

    OilParameters oil;
    PipeModelPGConstArea pipeModel(pipe, oil);

    profile_wrapper<double, 2> start_layer(get_profiles_pointers(buffer.current().vars.point_double));

    double G = 400;
    double Pout = 5e5;

    solve_euler_corrector<2>(pipeModel, -1, { Pout, G }, &start_layer);

    auto left_boundary = pipeModel.const_mass_flow_equation(G);
    auto right_boundary = pipeModel.const_pressure_equation(Pout);

    vector<vector<double>> Phist, Ghist;

    for (size_t index = 0; index < 100; ++index) {
        Phist.emplace_back(buffer.current().vars.point_double[0]);
        Ghist.emplace_back(buffer.current().vars.point_double[1]);

        buffer.advance(+1);

        auto left_boundary = pipeModel.const_mass_flow_equation(G + 50);

        moc_layer_wrapper<2> moc_current(buffer.current().vars, std::get<0>(buffer.current().specific));
        moc_layer_wrapper<2> moc_previous(buffer.previous().vars, std::get<0>(buffer.previous().specific));

        moc_solver<2> solver(pipeModel, moc_previous, moc_current);
        //double dt = 0.2;
        solver.step(left_boundary, right_boundary);

    }

}

