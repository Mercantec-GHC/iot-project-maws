using Blazor.Components;
using Microsoft.EntityFrameworkCore.Storage;
using Blazor.Services;
using DomainModels;
using API.Data;
using Microsoft.EntityFrameworkCore;

public class Program
{
    public static void Main(string[] args)
    {
        var builder = WebApplication.CreateBuilder(args);

        var connectionString = builder.Configuration.GetConnectionString("DefaultConnection")
            ?? Environment.GetEnvironmentVariable("DefaultConnection");

        // Configure DBContext
        builder.Services.AddDbContext<AppDBContext>(options => options.UseNpgsql(connectionString));

        // Register AppState
        builder.Services.AddScoped<AppState>();
        builder.Services.AddScoped<MailServices>();

        // Add services to the container.
        builder.Services.AddRazorComponents()
            .AddInteractiveServerComponents();

        // Configure HttpClient to connect to local API via HTTP
        builder.Services.AddHttpClient("API", client =>
        {
            // Use HTTP for local development to match Arduino requirements
            client.BaseAddress = new Uri("http://localhost:5021/");
            client.Timeout = TimeSpan.FromSeconds(30);
        });

        builder.Services.AddScoped(sp => sp.GetRequiredService<IHttpClientFactory>().CreateClient("API"));

        var app = builder.Build();

        // Configure the HTTP request pipeline.
        if (!app.Environment.IsDevelopment())
        {
            app.UseExceptionHandler("/Error");
            app.UseHsts();
        }

        // Only redirect to HTTPS for production
        if (!app.Environment.IsDevelopment())
        {
            app.UseHttpsRedirection();
        }

        app.UseStaticFiles();
        app.UseAntiforgery();
        app.MapRazorComponents<App>()
            .AddInteractiveServerRenderMode();

        app.Run();
    }
}